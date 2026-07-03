/**
 * @file angle.hpp
 * @author curl0z
 * @brief Degree<->Radian Angle conversions
 */

#pragma once

#include "vec3.hpp"

namespace clz::math
{
	constexpr float DEG_TO_RAD = 3.14159265358979323846 / 180;
	constexpr float RAD_TO_DEG = 180.0f / 3.14159265358979323846;

	/**
	 * @brief Converts degrees to radians
	 * @param degree angle in degrees
	 * @return angle in radians
	 */
	inline float radians(const float degree)
	{
		return degree * DEG_TO_RAD;
	}

	/**
	 * @brief Converts a vec3 of degrees to radians
	 * @param degree Vec3 degree
	 * @return angle Vec3 in radians
	 */
	inline vec3 radians(const vec3& degree)
	{
		return degree * DEG_TO_RAD;
	}

	/**
	 * @brief Converts radians to degrees
	 * @param rad angle in degrees
	 * @return angle in radians
	 */
	inline float degrees(const float rad)
	{
		return rad * RAD_TO_DEG;
	}

	/**
	 * @brief Converts a vec3 of radians to degrees
	 * @param rad Vec3 radians
	 * @return angle Vec3 in degrees
	 */
	inline vec3 degrees(const vec3& rad)
	{
		return rad * RAD_TO_DEG;
	}
} // namespace clz::math