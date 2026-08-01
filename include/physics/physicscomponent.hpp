#pragma once

#include "body.hpp"
#include "physics.hpp"

namespace clz::physics
{

/**
 * @brief Stores rigid body ID,
 * that can be used to query body's physical information
 *
 * @note Dependencies -> TransformComponent
 */
struct RigidBodyComponent
{
	RigidBodyId rigidBodyId = NULL_RIGID_BODY;
	math::quat prevRotation = math::quat();
	math::quat newRotation = math::quat();
	math::vec3 prevPosition = math::vec3(0.0f);
	math::vec3 newPosition = math::vec3(0.0f);

	RigidBodyComponent() = default;

	explicit RigidBodyComponent(
		const RigidBodyId rigidBodyId,
		const math::quat& prevRotation,
		const math::quat& newRotation,
		const math::vec3& prevPosition,
		const math::vec3& newPosition
	)
	    : rigidBodyId(rigidBodyId), prevRotation(prevRotation), newRotation(newRotation),
	      prevPosition(prevPosition), newPosition(newPosition)
	{
	}
};

} // namespace clz::physics