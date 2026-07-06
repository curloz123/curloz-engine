/**
 * @file math.hpp
 * @author curl0z
 * @brief Provides utility functions for conversion b/w
 * maths of box3D and clz::math
 */
#pragma once

#include <box3d/box3d.h>
#include "math/vec3.hpp"
#include "math/quat.hpp"

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
	 * @brief Converts clz::math::quat to b3Quat
	 *
	 * @param quat clz::math::quat Quaternion value
	 * @return b3Quat Quaternion value
	 */
	inline b3Quat toQuat(const math::quat& quat)
	{
		b3Quat q;
		q.s = quat.w;
		q.v = (b3Vec3){quat.x, quat.y, quat.z};
		return q;
	}
}