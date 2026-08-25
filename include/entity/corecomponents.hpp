/**
 * @file corecomponents.hpp
 * @author curl0z
 * @brief ECS core components definitions
 */

#pragma once

#include "math/quat.hpp"
#include "math/quateulerconv.hpp"
#include "math/vec3.hpp"

namespace clz::ecs
{

	/**
	 * @brief Stores position, rotation and scale of an entity in world space.
	 *
	 * Rotation is stored as quaternion
	 * This is the handshake between physics, renderer.
	 *
	 * @warning Do not change the layout without consulting the renderer and physics
	 * owners.
	 */
	struct TransformComponent
	{
		math::quat rotation = {1, 0, 0, 0}; ///< Quaternion rotation
		math::vec3 position = {0, 0, 0};    ///< World space position.
		math::vec3 scale = {1, 1, 1};	    ///< Non-uniform scale.

		TransformComponent() = default;

		TransformComponent(
			const math::quat& rotation,
			const math::vec3& position,
			const math::vec3& scale
		)
		    : rotation(rotation), position(position), scale(scale)
		{
		}
	};
	/**
	 * @brief Stores transform component as set by the editor.
	 *  This is necessary because after physics engine has calculated
	 *  and changed all the transform components **In Game Mode**, and
	 *  we go back to **Editor Mode**, we don't want the new physically
	 *  calculated transform do we??
	 *
	 *  @note Can only be changed by the editor, and initialized by scene at startup.
	 *  physics and rendering engine can only read from it.
	 *  No other subsystem is allowed to access it.
	 */
	struct EditorTransformComponent
	{
		math::vec3 rotation;
		math::vec3 position = {0, 0, 0}; ///< World space position.
		math::vec3 scale = {1, 1, 1};	 ///< Non-uniform scale.

		EditorTransformComponent()
		{
		}

		EditorTransformComponent(
			const math::vec3& rotation,
			const math::vec3& position,
			const math::vec3& scale
		)
		    : rotation(rotation), position(position), scale(scale)
		{
		}

		explicit EditorTransformComponent(const TransformComponent& transformComponent)
		    : rotation(math::quatToEulerXYZ(transformComponent.rotation)),
		      position(transformComponent.position), scale(transformComponent.scale)
		{
		}
	};

} // namespace clz::ecs