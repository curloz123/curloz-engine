/**
 * @file mat4x4.hpp
 * @brief SIMD accelerated 4x4 matrix
 * @author curl0z
 *
 * Uses SSE4.1 and FMA intrinsics
 */
#pragma once

#include "vec3.hpp"
#include <immintrin.h>

namespace clz::math
{
/**
 * @brief 16-component float vector backed by 4 128-bit SSE register.
 *
 * It's not recommented to access matrix data directly
 * All instances are 16-byte aligned for safe use with aligned SSE loads/stores.
 */
struct alignas(16) mat4
{
	union {
		struct
		{
			__m128 r0;
			__m128 r1;
			__m128 r2;
			__m128 r3;
		};
		struct
		{
			float xx, xy, xz, xw;
			float yx, yy, yz, yw;
			float zx, zy, zz, zw;
			float wx, wy, wz, ww;
		};
		float data[16];
	};

	mat4()
	    : r0(_mm_setzero_ps()), r1(_mm_setzero_ps()), r2(_mm_setzero_ps()), r3(_mm_setzero_ps())
	{
	}

	/// @brief Initializes Matrix in an identity format
	explicit mat4(const float value)
	    : r0(_mm_set_ps(0.0f, 0.0f, 0.0f, value)), r1(_mm_set_ps(0.0f, 0.0f, value, 0.0f)),
	      r2(_mm_set_ps(0.0f, value, 0.0f, 0.0f)), r3(_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f))
	{
	}

	explicit mat4(const clz::math::vec3& v)
	    : r0(_mm_set_ps(0.0f, 0.0f, 0.0f, v.x)), r1(_mm_set_ps(0.0f, 0.0f, v.y, 0.0f)),
	      r2(_mm_set_ps(0.0f, v.z, 0.0f, 0.0f)), r3(_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f))
	{
	}

	/// @brief Initializes the 4 __m128 registers
	mat4(__m128 r0, __m128 r1, __m128 r2, __m128 r3) : r0(r0), r1(r1), r2(r2), r3(r3)
	{
	}

	inline void makeIdentity()
	{
		r0 = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
		r1 = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
		r2 = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
		r3 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	}
	/**
	 * @brief Returns Addition of two matrices
	 * @param mat matrix 1
	 * @return Sum of m1 and m1 matrices
	 * Takes roughly ~15-20 cycles
	 */
	[[nodiscard]] mat4 operator+(const mat4& mat) const
	{
		mat4 result;

		result.r0 = _mm_add_ps(r0, mat.r0);
		result.r1 = _mm_add_ps(r1, mat.r1);
		result.r2 = _mm_add_ps(r2, mat.r2);
		result.r3 = _mm_add_ps(r3, mat.r3);

		return result;
	}

	/**
	 * @brief Multiplies the matrix with a scalar
	 * @param scalar float value
	 * @return resulting matrix
	 */
	mat4 operator*(const float scalar) const
	{
		mat4 result;

		result.r0 = _mm_mul_ps(result.r0, _mm_set1_ps(scalar));
		result.r1 = _mm_mul_ps(result.r1, _mm_set1_ps(scalar));
		result.r2 = _mm_mul_ps(result.r2, _mm_set1_ps(scalar));
		result.r3 = _mm_mul_ps(result.r3, _mm_set1_ps(scalar));

		return result;
	}

	/**
	 * @brief Multiplies two given matrices
	 * @param mat matrix 2
	 * @return this->mat4 X mat
	 *
	 * Takes around ~20-30 cycles
	 */
	inline mat4 operator*(const mat4& mat) const
	{
		mat4 result;

		// First row
		result.r3 = _mm_mul_ps(mat.r0, _mm_set1_ps(data[0]));
		result.r3 = _mm_fmadd_ps(mat.r1, _mm_set1_ps(data[1]), result.r3);
		result.r3 = _mm_fmadd_ps(mat.r2, _mm_set1_ps(data[2]), result.r3);
		result.r3 = _mm_fmadd_ps(mat.r3, _mm_set1_ps(data[3]), result.r3);
		result.r0 = result.r3;

		// second row
		result.r3 = _mm_mul_ps(mat.r0, _mm_set1_ps(data[4]));
		result.r3 = _mm_fmadd_ps(mat.r1, _mm_set1_ps(data[5]), result.r3);
		result.r3 = _mm_fmadd_ps(mat.r2, _mm_set1_ps(data[6]), result.r3);
		result.r3 = _mm_fmadd_ps(mat.r3, _mm_set1_ps(data[7]), result.r3);
		result.r1 = result.r3;

		// third row
		result.r3 = _mm_mul_ps(mat.r0, _mm_set1_ps(data[8]));
		result.r3 = _mm_fmadd_ps(mat.r1, _mm_set1_ps(data[9]), result.r3);
		result.r3 = _mm_fmadd_ps(mat.r2, _mm_set1_ps(data[10]), result.r3);
		result.r3 = _mm_fmadd_ps(mat.r3, _mm_set1_ps(data[11]), result.r3);
		result.r2 = result.r3;

		// fourth row
		result.r3 = _mm_mul_ps(mat.r0, _mm_set1_ps(data[12]));
		result.r3 = _mm_fmadd_ps(mat.r1, _mm_set1_ps(data[13]), result.r3);
		result.r3 = _mm_fmadd_ps(mat.r2, _mm_set1_ps(data[14]), result.r3);
		result.r3 = _mm_fmadd_ps(mat.r3, _mm_set1_ps(data[15]), result.r3);

		return result;
	}
};

/**
 * @brief Multiplies the matrix with a scalar
 * @param mat mat4 matrix
 * @param scalar float value
 * @return resulting matrix
 */
inline mat4 operator*(const mat4& mat, const float scalar)
{
	mat4 result = mat;

	result.r0 = _mm_mul_ps(mat.r0, _mm_set1_ps(scalar));
	result.r1 = _mm_mul_ps(mat.r1, _mm_set1_ps(scalar));
	result.r2 = _mm_mul_ps(mat.r2, _mm_set1_ps(scalar));
	result.r3 = _mm_mul_ps(mat.r3, _mm_set1_ps(scalar));

	return result;
}

/**
 * @brief Returns Addition of two matrices
 * @param m1 matrix 1
 * @param m2 matrix 2
 * @return Sum of m1 and m1 matrices
 * Takes roughly ~15-20 cycles
 */
inline mat4 add(const mat4& m1, const mat4& m2)
{
	return m1 + m2;
}

/**
 * @brief Multiplies two given matrices
 * @param m1 matrix 1
 * @param m2 matrix 2
 * @return M1 X M2
 *
 * Takes around ~20-30 cycles
 */
inline mat4 multiply(const mat4& m1, const mat4& m2)
{
	return m1 * m2;
}

} // namespace clz::math
