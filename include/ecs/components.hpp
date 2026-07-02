/**
 * @file components.hpp
 * @author curl0z
 * @brief ECS component definitions
 *
 * All components should be plain data structs.
 */

#pragma once

#include "math/quat.hpp"
#include "math/vec3.hpp"
#include "renderer/assets/modeldata.hpp"

namespace clz::ecs
{
	/**
	 * @brief Stores position, rotation and scale of an entity in world space.
	 *
	 * Rotation is stored as quaternion
	 * This is the handshake between ECS, physics, and the renderer.
	 *
	 * @warning Do not change the layout without consulting the renderer and physics owners.
	 */
	struct TransformComponent
	{
		clz::math::quat rotation = {1, 0, 0, 0};	///< Quaternion rotation
		clz::math::vec3 position = {0, 0, 0};		///< World space position.
		float pad0 = 0.0f;
		clz::math::vec3 scale	 = {1, 1, 1};		///< Non-uniform scale.
		float pad1 = 0.0f;

		TransformComponent() : rotation(math::quat(1.0f, 0.0f, 0.0f, 0.0f)),
					position(math::vec3(0.0f, 0.0f, 0.0f)),
					scale(math::vec3(1.0f, 1.0f, 1.0f))
		{
		}

		TransformComponent(const clz::math::quat& rotation, const clz::math::vec3& position,
				   const clz::math::vec3& scale): rotation(rotation),
								position(position), scale(scale)
		{
		}
	};

	/**
	 * @brief Euler rotation of transform. Required for editor
	 * @Dependencies -> TransformComponent
	 */
	struct EulerRotationComponent
	{
		math::vec3 rotation;
	};

	/**
	 * @brief Stores a renderable ModelID, that can be used to draw models
	 * @note Dependancies -> TransformComponent
	 */
	struct ModelComponent
	{
		clz::renderer::ModelID modelID;
		explicit ModelComponent(const clz::renderer::ModelID modelID)
			: modelID(modelID)
		{
		}
	};

} // namespace clz::ecs