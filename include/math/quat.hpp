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
		quat(const float w, const float x, const float y, const float z) : xmm(_mm_set_ps(z, y, x, w))
		{
		}

		/**
		 * @brief Initialies quaternions using a XMM register
		 * @param xmm XMM register containing the data
		 */
		explicit quat(const __m128 xmm) : xmm(xmm)
		{
		}

		/**
		 * @brief Operator overloaded function
		 * Of multiplication of two quaternions
		 *
		 * @param q Right hand side quaterion
		 * @return Multiplication of both quaternions
		 */
		quat operator*(const quat& q) const
		{
			return {w * q.w - x * q.x - y * q.y - x * q.z, w * q.x + x * q.w + y * q.z - z * q.y, w * q.y - x * q.z + y * q.w + z * q.x,
				w * q.z + x * q.y - y * q.x + z * q.w};
		}
	};

} // namespace clz::math