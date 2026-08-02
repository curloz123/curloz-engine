/**
 * @file quat.hpp
 * @author curl0z
 * @brief Quaternion Operation file
 * Contains all operations concerning quaternion
 */
#pragma once

#include <immintrin.h>

namespace clz::math
{
/**
 *  @brief Main quaternion struct
 */
struct quat
{
	/**
	 * A union, both 4-floats, and 4-float xmm instrinsic
	 * share the memory
	 */
	union {
		__m128 xmm;
		struct
		{
			float w;
			float x;
			float y;
			float z;
		};
	};

	/**
	 * @brief Identity initialize quaternion
	 */
	quat() : w(1.0f), x(0.0f), y(0.0f), z(0.0f)
	{
	}

	/**
	 * @brief Initializes quaternion with given 4 values
	 *
	 * @param w W component
	 * @param x X component
	 * @param y Y component
	 * @param z Z component
	 */
	quat(const float w, const float x, const float y, const float z)
	    : xmm(_mm_set_ps(z, y, x, w))
	{
	}

	/**
	 * @brief Initializes quaternions using a XMM register
	 * @param xmm XMM register containing the data
	 */
	explicit quat(const __m128 xmm) : xmm(xmm)
	{
	}

	/**
	 * @brief Operator overloaded function
	 * Of multiplication of two quaternions
	 *
	 * @param q Right hand side quaternion
	 * @return Multiplication of both quaternions
	 */
	quat operator*(const quat& q) const
	{
		return {w * q.w - x * q.x - y * q.y - z * q.z,
			w * q.x + x * q.w + y * q.z - z * q.y,
			w * q.y - x * q.z + y * q.w + z * q.x,
			w * q.z + x * q.y - y * q.x + z * q.w};
	}

	quat operator+(const quat& q) const
	{
		return {w + q.w, x + q.x, y + q.y, z + q.z};
	}
	quat operator-(const quat& q) const
	{
		return {w - q.w, x - q.x, y - q.y, z - q.z};
	}

	void normalize()
	{
		// 0xff = 1111 1111 - broadcast squared length into wxyz lanes for division
		const __m128 length = _mm_dp_ps(xmm, xmm, 0xff);
		xmm = _mm_div_ps(xmm, _mm_sqrt_ps(length));
	}

	void normalizeFast()
	{
		const __m128 length = _mm_dp_ps(xmm, xmm, 0xff);
		xmm = _mm_mul_ps(xmm, _mm_rsqrt_ps(length));
	}
};

inline quat operator*(const quat& q, const float s)
{
	return quat(_mm_mul_ps(q.xmm, _mm_set_ps(s, s, s, s)));
}
inline quat operator*(const float s, const quat& q)
{
	return quat(_mm_mul_ps(q.xmm, _mm_set_ps(s, s, s, s)));
}

inline float dot(const quat& a, const quat& b)
{
	const auto res = _mm_dp_ps(a.xmm, b.xmm, 0xf1);
	return _mm_cvtss_f32(res);
}

inline quat normalize(const quat& q)
{
	auto rq = q;
	rq.normalize();
	return rq;
}

} // namespace clz::math