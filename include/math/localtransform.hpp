/**
 * @file localtransform.hpp
 * @author curl0z
 * @brief Contains functions regarding Transform of any entity
 * I also don't know why I named it localtransform tbh
 */
#pragma once

#include "quat.hpp"
#include "vec3.hpp"
#include <cmath>

namespace clz::math
{
/**
 * @brief Scales a vector component-wise.
 * @param v Vector which has to be scale
 * @param scale Float scale value
 * @return Scaled Vector3
 */
inline vec3 scale(const vec3& v, const vec3& scale)
{
	return component_product(v, scale);
}

/**
 * @brief Translates a vector by an offset.
 * @param v Vector which has to be translated
 * @param translation Float translation value
 * @return Translated Vector3
 */
inline vec3 translate(const vec3& v, const vec3& translation)
{
	return add(v, translation);
}

/**
 * @brief Rotates a quaternion by an angle (radians) around an axis.
 * @param q Quaternion
 * @param rad Angle in radians
 * @param axis Axis about which to rotate
 * @note Use Axis.x()/y()/z() to determine axis, don't directly pass a vec3
 * @return Rotated Quaternion
 */
inline quat rotate(const quat& q, const float rad, const Axis& axis)
{
	const float half = rad * 0.5f;
	const float s = std::sin(half);
	const quat delta(std::cos(half), axis.x * s, axis.y * s, axis.z * s);
	return q * delta;
}

} // namespace clz::math
