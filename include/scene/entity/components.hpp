/**
 * @file components.hpp
 * @author curl0z
 * @brief ECS component definitions
 *
 * All components should be plain data structs.
 */

#pragma once

#include "../../renderer/entitydata/modeldata.hpp"
#include "math/quat.hpp"
#include "math/vec3.hpp"
#include "physics/body.hpp"
#include "physics/physics.hpp"
#include "physics/shape.hpp"

namespace clz::ecs
{
	/**
	 * @brief Stores position, rotation and scale of an entity in world space.
	 *
	 * Rotation is stored as quaternion
	 * This is the handshake between physics, renderer.
	 *
	 * @warning Do not change the layout without consulting the renderer and physics owners.
	 */
	struct TransformComponent
	{
		clz::math::quat rotation = {1, 0, 0, 0}; ///< Quaternion rotation
		clz::math::vec3 position = {0, 0, 0};	 ///< World space position.
		clz::math::vec3 scale = {1, 1, 1};	 ///< Non-uniform scale.

		TransformComponent()
		{
		}

		TransformComponent(const clz::math::quat& rotation, const clz::math::vec3& position, const clz::math::vec3& scale)
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
		clz::math::quat rotation = {1, 0, 0, 0}; ///< Quaternion rotation
		clz::math::vec3 position = {0, 0, 0};	 ///< World space position.
		clz::math::vec3 scale = {1, 1, 1};	 ///< Non-uniform scale.

		EditorTransformComponent()
		{
		}

		EditorTransformComponent(const clz::math::quat& rotation, const clz::math::vec3& position, const clz::math::vec3& scale)
		    : rotation(rotation), position(position), scale(scale)
		{
		}

		explicit EditorTransformComponent(const TransformComponent& transformComponent)
		    : rotation(transformComponent.rotation), position(transformComponent.position), scale(transformComponent.scale)
		{
		}
	};

	/**
	 * @brief Euler rotation of transform. Required for editor
	 * This is required because in editor we transform the rotation part via a Vector-3,
	 * whereas internally we store a quaternion, so just to ease the process we have this -_-
	 * @Dependencies -> TransformComponent
	 */
	struct EulerRotationComponent
	{
		math::vec3 rotation;
	};

	/**
	 * @brief Stores a renderable ModelID, that can be used to draw models
	 * @note Dependencies -> TransformComponent
	 */
	struct ModelComponent
	{
		clz::renderer::ModelID modelID;
		explicit ModelComponent(const clz::renderer::ModelID modelID) : modelID(modelID)
		{
		}
	};

	/**
	 * @brief Stores rigid body ID,
	 * that can be used to query body's physical information
	 *
	 * @note Dependencies -> TransformComponent
	 */
	struct BodyComponent
	{
		physics::BodyId bodyId;
		math::quat prevRotation;
		math::quat newRotation;
		math::vec3 prevPosition;
		math::vec3 newPosition;

		explicit BodyComponent()
		{
		}

		explicit BodyComponent(const physics::BodyId bodyId, const math::quat& prevRotation, const math::quat& newRotation,
				       const math::vec3& prevPosition, const math::vec3& newPosition)
		    : bodyId(bodyId), prevRotation(prevRotation), newRotation(newRotation), prevPosition(prevPosition), newPosition(newPosition)
		{
		}
	};
	/**
	 * @brief Stores the shapes attached to the body of current entity.
	 * Mainly used for editor purposes as body internally stores all shape handles too.
	 * Needed mainly to edit shapes etc etc.....
	 */
	struct ShapeComponent
	{
		std::vector<physics::BoxShape> boxShapes;
		ShapeComponent(const std::vector<physics::BoxShape>& boxShapes) : boxShapes(boxShapes)
		{
		}
	};

} // namespace clz::ecs