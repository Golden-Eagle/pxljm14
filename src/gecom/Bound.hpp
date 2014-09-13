/*
 * GECom Bound Header
 *
 *
 */

#ifndef GECOM_BOUND_HPP
#define GECOM_BOUND_HPP

#include <iostream>
#include <type_traits>

#include "GECom.hpp"

namespace gecom {

	template <typename> class aabb;
	using aabbf = aabb<float>;
	using aabbd = aabb<double>;

	template <typename T>
	inline i3d::vec3<T> positive_extremes(const i3d::vec3<T> &p0) {
		return p0;
	}

	// variadic positive_extremes
	template <typename TA, typename TB, typename... TR>
	inline auto positive_extremes(const i3d::vec3<TA> &p0, const i3d::vec3<TB> &p1, const i3d::vec3<TR> &...pr) ->
		i3d::vec3<typename std::common_type<TA, TB, TR...>::type>
	{
		return i3d::vec3<typename std::common_type<TA, TB, TR...>::type>::positive_extremes(p0, positive_extremes(p1, pr...));
	}

	template <typename T>
	inline i3d::vec3<T> negative_extremes(const i3d::vec3<T> &p0) {
		return p0;
	}

	// variadic negative_extremes
	template <typename TA, typename TB, typename... TR>
	inline auto negative_extremes(const i3d::vec3<TA> &p0, const i3d::vec3<TB> &p1, const i3d::vec3<TR> &...pr) ->
		i3d::vec3<typename std::common_type<TA, TB, TR...>::type>
	{
		return i3d::vec3<typename std::common_type<TA, TB, TR...>::type>::negative_extremes(p0, negative_extremes(p1, pr...));
	}

	// axis-aligned bounding box
	template <typename T>
	class aabb {
	public:
		using coord_t = T;
		using vec3_t = i3d::vec3<coord_t>;

	private:
		vec3_t m_center;
		vec3_t m_halfsize;

	public:
		inline aabb() { }

		inline aabb(const vec3_t &center_) : m_center(center_) { }

		inline aabb(const vec3_t &center_, const vec3_t &halfsize_) : m_center(center_) {
			m_halfsize.x() = std::abs(halfsize_.x());
			m_halfsize.y() = std::abs(halfsize_.y());
			m_halfsize.z() = std::abs(halfsize_.z());
		}

		template <typename TA>
		inline aabb(const aabb<TA> &other) : m_center(other.m_center), m_halfsize(other.m_halfsize) { }

		inline vec3_t center() const {
			return m_center;
		}

		inline vec3_t halfsize() const {
			return m_halfsize;
		}

		inline vec3_t min() const {
			return m_center - m_halfsize;
		}

		inline vec3_t max() const {
			return m_center + m_halfsize;
		}

		inline bool contains(const vec3_t &p) const {
			vec3_t dist = p - m_center;
			return (std::abs(dist.x()) <= m_halfsize.x()
				&& std::abs(dist.y()) <= m_halfsize.y()
				&& std::abs(dist.z()) <= m_halfsize.z());
		}

		inline bool contains(const aabb &a) const {
			vec3_t dist = a.m_center - m_center;
			vec3_t size_diff = m_halfsize - a.m_halfsize;
			return (std::abs(dist.x()) <= size_diff.x()
				&& std::abs(dist.y()) <= size_diff.y()
				&& std::abs(dist.z()) <= size_diff.z());
		}

		inline bool intersects(const aabb &a) const {
			vec3_t dist = a.m_center - m_center;
			vec3_t size_sum = m_halfsize + a.m_halfsize;
			return (std::abs(dist.x()) <= size_sum.x()
				&& std::abs(dist.y()) <= size_sum.y()
				&& std::abs(dist.z()) <= size_sum.z());
		}

		inline aabb & operator*=(const coord_t &rhs) {
			m_halfsize *= rhs;
			return *this;
		}

		inline aabb & operator/=(const coord_t &rhs) {
			m_halfsize /= rhs;
			return *this;
		}

		inline aabb operator*(const coord_t &rhs) const {
			return aabb(*this) *= rhs;
		}

		inline friend aabb operator*(const coord_t &lhs, const aabb &rhs) {
			return aabb(rhs) *= lhs;
		}

		inline aabb operator/(const coord_t &f) const {
			return aabb(*this) /= f;
		}

		// transform aabb by matrix
		inline friend aabb operator*(const i3d::mat4<coord_t> &lhs, const aabb &rhs) {
			vec3_t corners[8];
			// @josh - the original version of this most certainly wasnt getting the corners
			coord_t hsx = rhs.m_halfsize.x(), hsy = rhs.m_halfsize.y(), hsz = rhs.m_halfsize.z();
			corners[0] = lhs * (rhs.m_center + vec3_t(hsx, hsy, hsz));
			corners[1] = lhs * (rhs.m_center + vec3_t(hsx, hsy, -hsz));
			corners[2] = lhs * (rhs.m_center + vec3_t(hsx, -hsy, hsz));
			corners[3] = lhs * (rhs.m_center + vec3_t(hsx, -hsy, -hsz));
			corners[4] = lhs * (rhs.m_center + vec3_t(-hsx, hsy, hsz));
			corners[5] = lhs * (rhs.m_center + vec3_t(-hsx, hsy, -hsz));
			corners[6] = lhs * (rhs.m_center + vec3_t(-hsx, -hsy, hsz));
			corners[7] = lhs * (rhs.m_center + vec3_t(-hsx, -hsy, -hsz));

			vec3_t min(corners[0]);
			vec3_t max(corners[0]);

			for (int i = 1; i < 8; i++) {
				min = negative_extremes(min, corners[i]);
				max = positive_extremes(max, corners[i]);
			}

			return fromPoints(min, max);
		}

		inline friend std::ostream & operator<<(std::ostream &out, const aabb<T> &a) {
			out << "aabb[" << a.min() << " <= x <= " << a.max() << "]";
			return out;
		}

		inline static aabb fromPoints() {
			return aabb();
		}

		template <typename TA, typename... TR>
		inline static aabb fromPoints(const i3d::vec3<TA> &p0, const i3d::vec3<TR> &...pr) {
			vec3_t min = negative_extremes(p0, pr...);
			vec3_t max = positive_extremes(p0, pr...);
			vec3_t hs = (max - min) / 2;
			return aabb(min + hs, hs);
		}

	};

}

#endif