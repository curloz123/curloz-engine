/**
 * @file vec2.hpp
 * @brief SIMD accelerated 2-component float vector.
 * @author curl0z
 *
 * Uses SSE4.1 intrinsics via __m128 for all operations.
 * x86-64 only.
 */
#pragma once

#include <immintrin.h>

namespace clz::math
{
	/**
	 * @brief 2-component float vector backed by a 128-bit SSE register.
	 *
	 * Members x, y are accessible directly. It is not recommended to
	 * extract values via the underlying xmm register.
	 */
	struct vec2
	{
		float x, y;

		/// @brief Initializes all components to zero.
		vec2() : x(0.0f), y(0.0f)
		{
		}

		/**
		 * @brief Constructs a vec2 from two float values.
		 * @param x X component.
		 * @param y Y component.
		 */
		vec2(const float x, const float y) : x(x), y(y)
		{
		}

		/**
		 * @brief Constructs a vec2 directly from a __m128 register.
		 * @param xmm Source SSE register.
		 */
		vec2(const __m128& xmm)
		{
			x = _mm_cvtss_f32(xmm);
			__m128 res = _mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(1, 1, 1, 1));
			y = _mm_cvtss_f32(res);
		}
	};

	/**
	 * @brief Component-wise addition of two vec2s.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return lhs + rhs
	 */
	inline vec2 add(const vec2& lhs, const vec2& rhs)
	{
		return vec2(lhs.x + rhs.x, lhs.y + rhs.y);
	}

	/**
	 * @brief Component-wise subtraction of two vec2s.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return lhs - rhs
	 */
	inline vec2 subtract(const vec2& lhs, const vec2& rhs)
	{
		return vec2(lhs.x - rhs.x, lhs.y - rhs.y);
	}

	/**
	 * @brief Multiplies all components of a vec2 by a scalar.
	 * @param lhs Source vector.
	 * @param scalar Scalar multiplier.
	 * @return lhs * scalar
	 */
	inline vec2 scalar_product(const vec2& lhs, const float scalar)
	{
		return vec2(lhs.x * scalar, lhs.y * scalar);
	}

	/**
	 * @brief Component-wise multiplication of two vec2s.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return lhs * rhs (per component)
	 */
	inline vec2 component_product(const vec2& lhs, const vec2& rhs)
	{
		return vec2(lhs.x * rhs.x, lhs.y * rhs.y);
	}

	/**
	 * @brief Computes the dot product of two vec2s.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return Scalar dot product of lhs and rhs.
	 */
	inline float dot_product(const vec2& lhs, const vec2& rhs)
	{
		// 0x31 = 0011 0001 — multiply xy only, store in slot 0
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}

	/**
	 * @brief Computes the length (magnitude) of a vec2.
	 * @param v Source vector.
	 * @return Scalar length of v.
	 */
	inline float getLength(const vec2& v)
	{
		__m128 xmm = _mm_set_ps(0.0f, 0.0f, v.y, v.x);
		// dot(v, v) = squared length, then sqrt
		const __m128 sLength = _mm_dp_ps(xmm, xmm, 0x31);
		return _mm_cvtss_f32(_mm_sqrt_ss(sLength));
	}

	/**
	 * @brief Returns a normalized (unit length) copy of a vec2.
	 * @param v Source vector.
	 * @return v scaled to unit length.
	 * @note Uses _mm_rsqrt_ps — fast approximation with ~0.037% max error.
	 */
	inline vec2 normalize(const vec2& v)
	{
		__m128 xmm = _mm_set_ps(0.0f, 0.0f, v.y, v.x);
		// 0x33 = 0011 0011 — broadcast squared length into xy lanes for division
		const __m128 sLength = _mm_dp_ps(xmm, xmm, 0x33);
		return vec2(_mm_mul_ps(xmm, _mm_rsqrt_ps(sLength)));
	}

} // namespace clz::math
