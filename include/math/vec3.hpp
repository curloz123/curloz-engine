/**
 * @file vec3.hpp
 * @brief 3-component float vector.
 * @author curl0z
 *
 * x86-64 only.
 */
#pragma once

#include <immintrin.h>

namespace clz::math
{
	/**
	 * @brief 3-component float vector
	 *
	 * Members x, y, z are accessible directly
	 */
	struct vec3
	{
		float x, y, z;

		/// @brief Initializes all components to zero.
		vec3() : x(0.0f), y(0.0f), z(0.0f) {}

		/**
		 * @brief Constructs a vec3 from three float values.
		 * @param x X component.
		 * @param y Y component.
		 * @param z Z component.
		 */
		vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}

		/**
		 * @brief Constructs a vec3
		 * with x y z all set to a single float value
		 * @param value Initialize x,y,z to this value
		 */
		explicit vec3(const float value) : x(value), y(value), z(value) {}


		/**
		 * @brief Constructs a vec3 directly from a __m128 register.
		 * @param xmm Source SSE register.
		 */
		explicit vec3(const __m128& xmm)
		{
			x = _mm_cvtss_f32(xmm);
			__m128 res = _mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(1, 1, 1, 1));
			y = _mm_cvtss_f32(res);
			res = _mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(2, 2, 2, 2));
			z = _mm_cvtss_f32(res);
		}

		/**
		 * @brief Operator overloaded addition of current to rhs vector
		 * @param rhs Source SSE register.
		 */
		void operator += (const vec3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
		}
		/**
		 * @brief Operator overloaded subtraction of current to rhs vector
		 * @param rhs Source SSE register.
		 */
		void operator -= (const vec3& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
		}

		/**
		 * @brief Operator overloaded addition with another vector
		 * @param rhs Source SSE register.
		 */
		vec3 operator+(const vec3& rhs) const
		{
			return {x + rhs.x, y + rhs.y, z + rhs.z};
		}

		/**
		 * @brief Operator overloaded subtraction with another vector
		 * @param rhs Source SSE register.
		 */
		inline vec3 operator-(const vec3& rhs) const
		{
			return {x - rhs.x, y - rhs.y, z - rhs.z};
		}

		/**
		 * @brief Operator overloaded subtraction with a scalar
		 * @param scalar Source SSE register.
		 * @return Product of vector with scalar
		 */
		inline vec3 operator*(const float scalar) const
		{
			return {x * scalar, y * scalar, z * scalar};
		}
	};

	/**
	 * @brief Component-wise addition of two vec3's.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return lhs + rhs
	 */
	inline vec3 add(const vec3& lhs, const vec3& rhs)
	{
		return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
	}

	/**
	 * @brief Component-wise subtraction of two vec3's.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return lhs - rhs
	 */
	inline vec3 subtract(const vec3& lhs, const vec3& rhs)
	{
		return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
	}

	/**
	 * @brief Multiplies all components of a vec3 by a scalar.
	 * @param lhs Source vector.
	 * @param scalar Scalar multiplier.
	 * @return lhs * scalar
	 */
	inline vec3 scalar_product(const vec3& lhs, const float scalar)
	{
		return {lhs.x * scalar, lhs.y * scalar, lhs.z * scalar};
	}

	/**
	 * @brief Component-wise multiplication of two vec3's.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return lhs * rhs (per component)
	 */
	inline vec3 component_product(const vec3& lhs, const vec3& rhs)
	{
		return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
	}

	/**
	 * @brief Computes the dot product of two vec3s.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return Scalar dot product of lhs and rhs.
	 */
	inline float dot(const vec3& lhs, const vec3& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	/**
	 * @brief Returns cross product of two vectors
	 * @note 1st parameter is considered left hand side
	 * @param lhs left hand side vector
	 * @param rhs right hand side vector
	 * @return cross of both vectors
	 */
	inline vec3 cross(const vec3& lhs, const vec3& rhs)
	{
		return
		{
			lhs.y * rhs.z - lhs.z * rhs.y,
			lhs.z * rhs.x - lhs.x * rhs.z,
			lhs.x * rhs.y - lhs.y * rhs.x
		};
	}

	/**
	 * @brief Computes the length (magnitude) of a vec3.
	 * @param lhs Source vector.
	 * @return Scalar length of lhs.
	 */
	inline float getLength(const vec3& lhs)
	{
		__m128 vec = _mm_set_ps(0, lhs.z, lhs.y, lhs.x);
		const __m128 lSquare = _mm_dp_ps(vec, vec, 0x71);
		return _mm_cvtss_f32(_mm_sqrt_ss(lSquare));
	}


	/**
	 * @brief Computes the square length (magnitude) of a vec3.
	 * Useful in places to check if length != 0
	 * @param lhs Source vector.
	 * @return Scalar length of lhs.
	 */
	inline float getLengthSquared(const vec3& lhs)
	{
		__m128 vec = _mm_set_ps(0, lhs.z, lhs.y, lhs.x);
		const __m128 lSquare = _mm_dp_ps(vec, vec, 0x71);
		return _mm_cvtss_f32(lSquare);
	}

	/**
	 * @brief Returns a normalized (unit length) copy of a vec3.
	 * @param lhs Source vector.
	 * @return lhs scaled to unit length.
	 * @note Uses _mm_rsqrt_ps. Fast approximation but
	 * not recommended if you want high precision
	 */
	inline vec3 normalize(const vec3& lhs)
	{
		__m128 vec = _mm_set_ps(0, lhs.z, lhs.y, lhs.x);
		// 0x77 = 0111 0111 — broadcast squared length into xyz lanes for division
		const __m128 length = _mm_dp_ps(vec, vec, 0x77);
		return vec3(_mm_mul_ps(vec, _mm_rsqrt_ps(length)));
	}

	/**
	 * @brief Describes Axis - X, Y, Z
	 * Useful because passing a vec3 raw to any function is kinda dangerous
	 * Just use this for axis related operations(dw we enforce it anyways)
	 */
	struct Axis
	{
		union
		{
			vec3 vector;
			struct { float x, y, z; };
		};

		Axis(const float a, const float b, const float c)
		: vector(vec3(a, b, c))
		{
		}

		static Axis X()
		{
			return Axis(1.0f, 0.0f, 0.0f);
		}
		static Axis Y()
		{
			return Axis(0.0f, 1.0f, 0.0f);
		}
		static Axis Z()
		{
			return Axis(0.0f, 0.0f, 1.0f);
		}
	};
} // namespace clz::math
