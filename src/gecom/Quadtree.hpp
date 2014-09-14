/*
 * GECom Quadtree Header
 *
 *
 */

#ifndef GECOM_QUADTREE_HPP
#define GECOM_QUADTREE_HPP

#define GECOM_QUADTREE_MAX_LEAF_ELEMENTS 8

#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <utility>

#include "GECom.hpp"
#include "Bound.hpp"

namespace gecom {

	// quadtree. uses an unordered_map for values at each node, so value type must be hashable.
	template <typename T, typename CoordT = double, typename HashT = std::hash<T>, typename EqualToT = std::equal_to<T>>
	class quadtree {
	public:
		using value_t = T;
		using coord_t = CoordT;
		using vec3_t = i3d::vec3<coord_t>;
		using aabb_t = aabb<coord_t>;
		using hash_t = HashT;
		using equal_to_t = EqualToT;
		using found_t = std::function<void(const value_t &, const aabb_t &)>;

	private:
		using map_t = std::unordered_map<T, aabb_t, HashT, EqualToT>;

		struct out_of_bounds { };

		class Node {
		private:
			aabb_t m_bound;
			Node *m_children[4];
			map_t m_values;
			size_t m_count = 0;
			bool m_isleaf = true;

			inline void dump(map_t &values) {
				// move values out of this node
				for (auto it = m_values.begin(); it != m_values.end(); ++it) {
					values.insert(std::move(*it));
				}
				// move values out of child nodes
				for (Node **pn = m_children + 4; pn --> m_children; ) {
					if (*pn) (*pn)->dump(values);
				}
				// safety
				m_values.clear();
				m_count = 0;
			}

			inline unsigned childID(const vec3_t &p) {
				const vec3_t c = m_bound.center();
				unsigned cid = 0;
				cid |= unsigned(p.x() >= c.x()) << 0;
				cid |= unsigned(p.y() >= c.y()) << 1;
				return cid;
			}
			
			inline void unleafify();

			inline void leafify();

		public:
			inline Node(const aabb_t &a_) : m_bound(a_) {
				// clear children pointers
				std::memset(m_children, 0, 4 * sizeof(Node *));
			}

			// no copy assignment
			Node & operator=(const Node &other) = delete;

			inline Node(const Node &other) :
				m_bound(other.m_bound),
				m_values(other.m_values),
				m_count(other.m_count),
				m_isleaf(other.m_isleaf)
			{
				// clear children pointers
				std::memset(m_children, 0, 4 * sizeof(Node *));
				// make new child nodes
				for (int i = 0; i < 4; i++) {
					if (other.m_children[i]) {
						m_children[i] = new Node(*(other.m_children[i]));
					}
				}
			}

			inline aabb_t bound() {
				return m_bound;
			}

			inline size_t count() {
				return m_count;
			}

			inline size_t countRecursively() {
				size_t c = m_values.size();
				for (Node **pn = m_children + 4; pn --> m_children; ) {
					if (*pn) c += (*pn)->countRecursively();
				}
				return c;
			}

			// insert a value, using a specified bounding box
			inline bool insert(const T &t, const aabb_t &a) {
				T t2(t);
				return insert(std::move(t2), a);
			}

			// move-insert a value, using a specified bounding box
			inline bool insert(T &&t, const aabb_t &a) {
				if (!m_bound.contains(a)) throw out_of_bounds();
				if (m_isleaf && m_count < GECOM_QUADTREE_MAX_LEAF_ELEMENTS) {
					if (!m_values.insert(std::move(std::make_pair(std::move(t), a))).second) return false;
				} else {
					// not a leaf or should not be
					unleafify();
					unsigned cid_min = childID(a.min());
					unsigned cid_max = childID(a.max());
					if (cid_min != cid_max) {
						// element spans multiple child nodes, insert into this one
						if (!m_values.insert(std::move(std::make_pair(std::move(t), a))).second) return false;
					} else {
						// element contained in one child node - create if necessary then insert
						Node *child = m_children[cid_min];
						if (!child) {
							const vec3_t c = m_bound.center();
							// vector to a corner of the current node's aabb
							vec3_t vr = m_bound.halfsize();
							if (!(cid_min & 0x1)) vr.x() = -vr.x();
							if (!(cid_min & 0x2)) vr.y() = -vr.y();
							child = new Node(aabb_t::fromPoints(c, c + vr));
							m_children[cid_min] = child;
						}
						try {
							if (!child->insert(std::move(t), a)) return false;
						} catch (out_of_bounds &e) {
							// child doesn't want to accept it
							if (!m_values.insert(std::move(std::make_pair(std::move(t), a))).second) return false;
						}
					}
				}
				m_count++;
				return true;
			}

			// erase a value, searching within a specified bounding box
			// (can be different to the one it was added with)
			inline bool erase(const T &t, const aabb_t &a) {
				if (!m_bound.intersects(a)) return false;
				unsigned cid_min = childID(a.min());
				unsigned cid_max = childID(a.max());
				auto it = m_values.find(t);
				if (it != m_values.end()) {
					// value is in this node
					if (a.intersects(it->second)) {
						// must intersect search box to be erased
						m_values.erase(it);
						return true;
					}
					return false;
				} else {
					if (m_isleaf) return false;
					Node *child;
					if (cid_min != cid_max) {
						// search is split across children => try erase from all
						for (Node **pn = m_children + 4; pn --> m_children; ) {
							if (*pn) {
								if ((*pn)->erase(t, a)) {
									// found, stop searching
									child = *pn;
									break;
								}
							}
						}
						// not found in any child
						if (!child) return false;
					} else {
						// erase from one
						child = m_children[cid_min];
						if (!child) return false;
						if (!child->erase(t, a)) return false;
					}
					if (child->count() == 0) {
						// child is now empty, delete it
						m_children[cid_min] = nullptr;
						delete child;
					}
				}
				m_count--;
				if (m_count <= GECOM_QUADTREE_MAX_LEAF_ELEMENTS) leafify();
				return true;
			}

			// test if this node contains a value, searching within a specified bounding box
			// (can be different to the one it was added with)
			inline bool contains(const T &t, const aabb_t &a) {
				if (!m_bound.intersects(a)) return false;
				if (m_values.find(t) != m_values.end()) return true;
				if (m_isleaf) return false;
				unsigned cid_min = childID(a.min());
				unsigned cid_max = childID(a.max());
				if (cid_min != cid_max) {
					// search is split across children => search all
					for (Node **pn = m_children + 4; pn --> m_children; ) {
						if (*pn) {
							if ((*pn)->contains(t, a)) {
								// found, stop searching
								return true;
							}
						}
					}
				} else {
					// search in one
					Node *child = m_children[cid_min];
					if (!child) return false;
					return child->contains(t, a);
				}
			}

			// search this node for values intersecting the specified bounding box
			inline void search(const aabb_t &a, const found_t &found) {
				if (m_bound.intersects(a)) {
					for (auto it = m_values.begin(); it != m_values.end(); ++it) {
						if (a.intersects(it->second)) found(it->first, it->second);
					}
					if (m_isleaf) return;
					unsigned cid_min = childID(a.min());
					unsigned cid_max = childID(a.max());
					if (cid_min != cid_max) {
						// search is split across children => search all
						for (Node **pn = m_children + 4; pn --> m_children; ) {
							if (*pn) (*pn)->search(a, found);
						}
					} else {
						// search intersects only one child
						Node *child = m_children[cid_min];
						if (child) child->search(a, found);
					}
				}
			}

			// directly add a child node. minimal safety checking.
			inline void putChild(Node *child) {
				unsigned cid = childID(child->bound().center());
				assert(!m_children[cid]);
				m_children[cid] = child;
				m_count += child->count();
				// need to ensure the child is added properly
				unleafify();
				if (m_count <= GECOM_QUADTREE_MAX_LEAF_ELEMENTS) {
					// if this should actually be a leaf after all
					leafify();
				}
			}

			inline void print(const std::string &indent = "") {
				std::cout << indent << m_bound << " (node)" << std::endl;
				for (auto it = m_values.begin(); it != m_values.end(); ++it) {
					std::cout << indent << "    " << it->second << " : " << it->first << std::endl;
				}
				for (Node **pn = m_children + 4; pn --> m_children; ) {
					if (*pn) (*pn)->print(indent + "    ");
				}
			}

			inline size_t height() {
				size_t h = 0;
				for (Node **pn = m_children + 4; pn --> m_children; ) {
					if (*pn) {
						size_t hn = (*pn)->height();
						h = initial3d::math::max(h, hn);
					}
				}
				return h + 1;
			}

			inline double heightAvg() {
				double h = 0;
				int c = 0;
				for (Node **pn = m_children + 4; pn --> m_children; ) {
					if (*pn) {
						h += (*pn)->heightAvg();
						c++;
					}
				}
				return 1 + (c > 0 ? h / c : 0);
			}

			// iterators...

			inline ~Node() {
				// delete child nodes
				for (Node **pn = m_children + 4; pn --> m_children; ) {
					if (*pn) delete *pn;
				}
			}

		};

		Node *m_root = nullptr;
		
		// kill the z dimension of an aabb so this actually functions as a quadtree
		static inline aabb_t sanitize(const aabb_t &a) {
			vec3_t c = a.center();
			vec3_t h = a.halfsize();
			c.z() = 0;
			h.z() = 0;
			return aabb_t(c, h);
		}

		inline void destroy() {
			if (m_root) delete m_root;
			m_root = nullptr;
		}

	public:
		inline quadtree() { }
		
		inline quadtree(const aabb_t &rootbb) {
			m_root = new Node(sanitize(rootbb));
		}
		
		inline quadtree(const quadtree &other) {
			if (other.m_root) m_root = new Node(other.m_root);
		}
		
		inline quadtree(quadtree &&other) {
			m_root = other.m_root;
			other.m_root = nullptr;
		}
		
		inline quadtree & operator=(const quadtree &other) {
			destroy();
			if (other.m_root) m_root = new Node(other.m_root);
			return *this;
		}
		
		inline quadtree & operator=(quadtree &&other) {
			m_root = other.m_root;
			other.m_root = nullptr;
		}
		
		inline bool insert(const T &value, const aabb_t &valuebb) {
			T t2(value);
			return insert(std::move(t2), valuebb);
		}

		inline bool insert(T &&value, const aabb_t &valuebb) {
			aabb_t valuebb2 = sanitize(valuebb);
			if (!m_root) m_root = new Node(valuebb2);
			try {
				return m_root->insert(std::move(value), valuebb2);
			} catch (out_of_bounds &) {
				// make new root
				Node * const oldroot = m_root;
				const aabb_t a = m_root->bound();
				// vector from centre to max of new root
				const vec3_t vr = 2.0 * a.halfsize();
				// vector from centre of current root to centre of new element
				const vec3_t vct = valuebb2.center() - a.center();
				// vector from current root to corner nearest centre of new element
				vec3_t corner = vr * 0.5;
				if (vct.x() < 0) corner.x() = -corner.x();
				if (vct.y() < 0) corner.y() = -corner.y();
				// centre of new root
				const vec3_t newcentre = a.center() + corner;
				m_root = new Node(sanitize(aabb_t(newcentre, vr)));
				if (oldroot->count() > 0) {
					// only preserve old root if it had elements
					m_root->putChild(oldroot);
				} else {
					delete oldroot;
				}
				// re-attempt to add
				return insert(std::move(value), valuebb2);
			}
		}
		
		inline bool erase(const T &value, const aabb_t &searchbb) {
			if (!m_root) return false;
			// TODO root reduction
			return m_root->erase(value, sanitize(searchbb));
		}
		
		inline bool contains(const T &value, const aabb_t &searchbb) const {
			if (!m_root) return false;
			return m_root->contains(value, sanitize(searchbb));
		}
		
		inline void search(const aabb_t &searchbb, const found_t &found) const {
			if (!m_root) return;
			m_root->search(sanitize(searchbb), found);
		}
		
		inline std::vector<value_t> search(const aabb_t &searchbb) const {
			std::vector<value_t> r;
			search(searchbb, [&](const value_t &v, const aabb_t &) {
				r.push_back(v);
			});
			return r;
		}
		
		inline void print() const {
			if (!m_root) return;
			m_root->print();
		}
		
		inline size_t height() const {
			if (!m_root) return 0;
			return m_root->height();
		}
		
		inline double heightAvg() const {
			if (!m_root) return 0;
			return m_root->heightAvg();
		}
		
		inline size_t count() const {
			if (!m_root) return 0;
			return m_root->count();
		}
		
		inline size_t countRecursively() const {
			if (!m_root) return 0;
			return m_root->countRecursively();
		}
		
		inline bool empty() {
			return !count();
		}
		
		inline ~quadtree() {
			destroy();
		}

	};
	
	template <typename T, typename CoordT, typename HashT, typename EqualToT>
	inline void quadtree<T, CoordT, HashT, EqualToT>::Node::unleafify() {
		if (m_isleaf) {
			m_isleaf = false;
			// move current elements out of node
			map_t temp = std::move(m_values);
			// decrement the count because re-inserting will increment it again
			m_count -= temp.size();
			// re-insert values
			for (auto it = temp.begin(); it != temp.end(); ++it) {
				insert(std::move(it->first), it->second);
			}
		}
	}
	
	template <typename T, typename CoordT, typename HashT, typename EqualToT>
	inline void quadtree<T, CoordT, HashT, EqualToT>::Node::leafify() {
		if (!m_isleaf) {
			m_isleaf = true;
			// dump values in child nodes into this one
			for (Node **pn = m_children + 4; pn --> m_children; ) {
				if (*pn) {
					(*pn)->dump(m_values);
					delete *pn;
					*pn = nullptr;
				}
			}
		}
	}

}

#endif
