/**
 * @file rigidbodycomponent.hpp
 * @author curl0z
 * @brief displays model component's data n all
 * in inspector. Internally can change rigid body's
 * data and body's attributes.
 */

#pragma once

#include "entity/entitymanager.hpp"
#include "renderer/shapes.hpp"
#include <optional>
#include "physics/body.hpp"
#include "physics/shape.hpp"

namespace clz::editor
{
	/// @brief Captures the rigid body id of entity selected atm.

	struct EditorBodyData
	{
		std::optional<ecs::entity> entityId;
		physics::BodyType type;
		bool enableSleep;
		math::vec3 position;
		math::quat rotation;
		float linearDamping;
		float angularDamping;
		std::array<bool, 3> linearLocks;
		std::array<bool, 3> angularLocks;

		std::vector<physics::ShapeDef> shapeData;

		void clearData()
		{
			entityId.reset();
			shapeData.clear();
		}
	};
	inline EditorBodyData capturedBodyData;

	/// @brief Draws the RigidBody section of the Inspector for the selected entity.
	void showRigidBodyHeader();

	/// @brief Renders the selected entity's model and pending/committed box
	/// shapes into the physics body-shape offscreen target.
	/// @param commandBuffer Active command buffer, must be in recording state.
	void drawBodyEditorOffscreenImage(VkCommandBuffer commandBuffer);

	/// @brief Draws the "Shape Controls" and "RigidBody Shape Editor" ImGui windows.
	void presentBodyEditorWindow();

}