/**
 * @file interpolate.hpp
 * @author curl0z
 * @brief Math namespace for Curloz Engine.
 */
#pragma once

#include "quat.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <cmath>

namespace clz::math
{
	/**
	 * @brief Linearly interpolates between two 3D vectors.
	 *
	 * @param begin The starting vector.
	 * @param end The ending vector.
	 * @param alpha The interpolation factor.
	 * @return The interpolated vector.
	 */
	inline math::vec3 lerp(const vec3& begin, const vec3& end, const float alpha)
	{
		return begin + (end - begin) * alpha;
	}

	/**
	 * @brief Spherically linearly interpolates between two quaternions.
	 *
	 * @param begin The starting quaternion.
	 * @param end The ending quaternion.
	 * @param alpha The interpolation factor.
	 * @return The interpolated quaternion.
	 */
	inline math::quat slerp(const math::quat& begin, const math::quat& end, const float alpha)
	{
		float d = dot(begin, end);
		float negate = 1.0f;
		if (d < 0.0f)
		{
			negate = -1.0f;
			d = -d;
		}
		const float cosine = std::clamp(d, -1.0f, 1.0f);

		if (cosine > 0.9995f)
		{
			return normalize(begin + ((end * negate) - begin) * alpha);
		}

		const float angle = std::acos(cosine);

		const quat q1 = (std::sin((1 - alpha) * angle)) / std::sin(angle) * begin;
		const quat q2 = (std::sin(alpha * angle)) / std::sin(angle) * (end * negate);
		return normalize(q1 + q2);
	}
} // namespace clz::math