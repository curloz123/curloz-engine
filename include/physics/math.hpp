/**
 * @file math.hpp
 * @author curl0z
 * @brief Provides utility functions for conversion b/w
 * maths of box3D and clz::math
 */
#pragma once

#include "math/quat.hpp"
#include "math/vec3.hpp"
#include <box3d/box3d.h>

namespace clz::physics
{
	/**
	 * @brief Converts clz::math::vec3 to b3Vec3
	 *
	 * @param vec clz::math::vec3 vector3 value
	 * @return b3Vec3 vector3 value
	 */
	inline b3Vec3 toVec3(const math::vec3& vec)
	{
		return (b3Vec3){vec.x, vec.y, vec.z};
	}

	/**
	 * @brief converts physics engine's internal vec3 data type
	 * to engine's native.
	 * @param vec Physics engine's internal vec3 data type
	 * @return equivalent math::vec3
	 */
	inline math::vec3 fromVec3(const b3Vec3& vec)
	{
		return {vec.x, vec.y, vec.z};
	}

	/**
	 * @brief Converts clz::math::quat to b3Quat
	 *
	 * @param quat clz::math::quat Quaternion value
	 * @return b3Quat Quaternion value
	 */
	inline b3Quat toQuat(const math::quat& quat)
	{
		return b3Quat{
		    .v = (b3Vec3){quat.x, quat.y, quat.z},
		    .s = quat.w,
		};
	}

	/**
	 * @brief converts physics engine's internal quaternion data type
	 * to engine's native.
	 * @param quat Physics engine's internal vec3 data type
	 * @return equivalent math::vec3
	 */
	inline math::quat fromQuat(const b3Quat& quat)
	{
		const auto [x, y, z] = quat.v;
		return {quat.s, x, y, z};
	}

} // namespace clz::physics