/**
 * @file vec4.hpp
 * @brief SIMD accelerated 4-component float vector.
 * @author curl0z
 *
 * Uses SSE4.1 intrinsics via __m128 for all operations.
 * Requires 16-byte alignment — guaranteed by alignas(16).
 * x86-64 only.
 */
#pragma once

#include <immintrin.h>

namespace clz::math
{
/**
 * @brief 4-component float vector backed by a 128-bit SSE register.
 *
 * Members x, y, z, w are accessible directly
 * not recommended to retrieve values via the underlying xmm register.
 * All instances are 16-byte aligned for safe use with aligned SSE loads/stores.
 */
struct alignas(16) vec4
{
	union {
		__m128 xmm;
		struct
		{
			float x, y, z, w;
		};
	};

	/// @brief Initializes all components to zero.
	vec4() : xmm(_mm_setzero_ps())
	{
	}

	/**
	 * @brief Constructs a vec4 from four float values.
	 * @param x X component.
	 * @param y Y component.
	 * @param z Z component.
	 * @param w W component.
	 */
	vec4(const float x, const float y, const float z, const float w)
	    : xmm(_mm_set_ps(w, z, y, x))
	{
	}

	/**
	 * @brief Constructs a vec4 directly from a __m128 register.
	 * @param xmm Source SSE register.
	 */
	vec4(const __m128 xmm) : xmm(xmm)
	{
	}

	/**
	 * @brief operator overloaded Component-wise addition of two vec4s.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return lhs + rhs
	 */
	inline vec4 operator+(const vec4& otherVec) const
	{
		return vec4(_mm_add_ps(xmm, otherVec.xmm));
	}

	/**
	 * @brief operator overloaded Component-wise subtraction of two vec4s.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return lhs - rhs
	 */
	inline vec4 operator-(const vec4& otherVec) const
	{
		return vec4(_mm_sub_ps(xmm, otherVec.xmm));
	}

	/**
	 * @brief Operator overloaded Multiplies all components of a vec4 by a scalar.
	 * @param lhs Source vector.
	 * @param scalar Scalar multiplier.
	 * @return lhs * scalar
	 */
	inline vec4 operator*(const float& scalar) const
	{
		return vec4(_mm_mul_ps(xmm, _mm_set1_ps(scalar)));
	}

	/**
	 * @brief operator overloaded Component-wise multiplication of two vec4s.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return lhs * rhs (per component)
	 */
	inline vec4 operator*(const vec4& otherVec) const
	{
		return vec4(_mm_mul_ps(xmm, otherVec.xmm));
	}
};

/**
 * @brief Component-wise addition of two vec4s.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return lhs + rhs
 */
inline vec4 add(const vec4& lhs, const vec4& rhs)
{
	return lhs + rhs;
}

/**
 * @brief Component-wise subtraction of two vec4s.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return lhs - rhs
 */
inline vec4 subtract(const vec4& lhs, const vec4& rhs)
{
	return lhs - rhs;
}

/**
 * @brief Multiplies all components of a vec4 by a scalar.
 * @param lhs Source vector.
 * @param scalar Scalar multiplier.
 * @return lhs * scalar
 */
inline vec4 scalar_product(const vec4& lhs, const float scalar)
{
	return lhs * scalar;
}

/**
 * @brief Component-wise multiplication of two vec4s.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return lhs * rhs (per component)
 */
inline vec4 component_product(const vec4& lhs, const vec4& rhs)
{
	return lhs * rhs;
}

/**
 * @brief Computes the dot product of two vec4s.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Scalar dot product of lhs and rhs.
 */
inline float dot_product(const vec4& lhs, const vec4& rhs)
{
	// 0xf1 = 1111 0001 — multiply all 4 lanes, store in slot 0
	const auto res = _mm_dp_ps(lhs.xmm, rhs.xmm, 0xf1);
	return _mm_cvtss_f32(res);
}

} // namespace clz::math
