/**
 * @file components.hpp
 * @author curl0z
 * @brief ECS component definitions
 *
 * All components should be plain data structs.
 */

#pragma once

#include "math/vec3.hpp"
#include <AL/al.h>

namespace clz::ecs
{
	/**
	 * @brief Stores position, rotation and scale of an entity in world space.
	 *
	 * Rotation is stored as Euler angles in degrees.
	 * This is the handshake between ECS, physics, and the renderer.
	 *
	 * @warning Do not change the layout without consulting the renderer and physics owners.
	 */
	struct TransformComponent
	{
		clz::math::vec3 position = {0, 0, 0};	///< World space position.
		clz::math::vec3 rotation = {0, 0, 0};	///< Euler angles in degrees (pitch, yaw, roll).
		clz::math::vec3 scale	 = {1, 1, 1};	///< Non-uniform scale.

		TransformComponent(const clz::math::vec3 &position, const clz::math::vec3 &rotation,
					const clz::math::vec3 &scale) : position(position), rotation(rotation),
									scale(scale) {}

	};

	struct AudioBuffer
	{
		ALuint bufferID;
	};

	struct AudioSource 
	{
		ALuint sourceID;
	};

} // namespace clz::ecs
