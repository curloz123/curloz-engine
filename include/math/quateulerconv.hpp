#pragma once

#include "quat.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <cmath>

namespace clz::math
{
inline quat quatFromEuler(const vec3& euler)
{
	const float cx = cosf(euler.x * 0.5f);
	const float sx = sinf(euler.x * 0.5f);

	const float cy = cosf(euler.y * 0.5f);
	const float sy = sinf(euler.y * 0.5f);

	const float cz = cosf(euler.z * 0.5f);
	const float sz = sinf(euler.z * 0.5f);

	return {cx * cy * cz + sx * sy * sz,
		sx * cy * cz - cx * sy * sz,
		cx * sy * cz + sx * cy * sz,
		cx * cy * sz - sx * sy * cz};
}

inline vec3 quatToEulerXYZ(const quat& q)
{
	vec3 euler;

	const float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
	const float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	euler.x = std::atan2(sinr_cosp, cosr_cosp);

	const float sinp = 2.0f * (q.w * q.y - q.z * q.x);
	if (std::abs(sinp) >= 1.0f)
		euler.y = std::copysign(std::numbers::pi_v<float> / 2.0f, sinp);
	else
		euler.y = std::asin(sinp);

	const float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
	const float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	euler.z = std::atan2(siny_cosp, cosy_cosp);

	return euler;
}
} // namespace clz::math