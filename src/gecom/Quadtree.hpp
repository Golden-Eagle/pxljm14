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
		using found_t = std::function<void(const value_t &)>;

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
				values.insert(m_values.begin(), m_values.end());
				for (Node **pn = m_children + 4; pn --> m_children; ) {
					if (*pn) (*pn)->dump(values);
				}
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
				if (!m_bound.contains(a)) throw out_of_bounds();
				if (m_isleaf && m_count < GECOM_QUADTREE_MAX_LEAF_ELEMENTS) {
					if (!m_values.insert(t).second) return false;
				} else {
					// not a leaf or should not be
					unleafify();
					unsigned cid_min = childID(a.min());
					unsigned cid_max = childID(a.max());
					if (cid_min != cid_max) {
						// element spans multiple child nodes, insert into this one
						if (!m_values.insert(std::make_pair(t, a)).second) return false;
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
							if (!child->insert(t, a)) return false;
						} catch (out_of_bounds &e) {
							// child doesn't want to accept it
							if (!m_values.insert(std::make_pair(t, a)).second) return false;
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
						if (a.intersects(it->second)) found(it.first);
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
				unleafify();
			}

			inline void print(const std::string &indent) {
				std::cout << indent << m_bound << std::endl;
				for (auto it = m_values.begin(); it != m_values.end(); ++it) {
					std::cout << indent << "    " << **it << std::endl;
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

		Node *m_root;

	public:

	};

}

#endif