/**
 * @file rigidbodycomponent.hpp
 * @author curl0z
 * @brief displays model component's data n all
 * in inspector. Internally can change rigid body's
 * data and body's attributes.
 */

#pragma once

#include <vector>
#include "renderer/shapes.hpp"
#include "physics/shape.hpp"
#include <set>

namespace clz::editor
{
	/// @brief Draws the RigidBody section of the Inspector for the selected entity.
	void showRigidBodyHeader();

	/// @brief Renders the selected entity's model and pending/committed box
	/// shapes into the physics body-shape offscreen target.
	/// @param commandBuffer Active command buffer, must be in recording state.
	void drawBodyEditorOffscreenImage(VkCommandBuffer commandBuffer);

	/// @brief Draws the "Shape Controls" and "RigidBody Shape Editor" ImGui windows.
	void presentBodyEditorWindow();

	/// @brief True while there are uncommitted shape edits pending (new or modified) that haven't been saved.
	inline bool anyChanges = false;

	/// @brief Indices into changedShapes whose local position/rotation/dimensions were edited this session.
	inline std::set<uint32_t> changedShapesIndex;

	/// @brief Working copy of the entity's existing box shapes, edited in place before Save.
	inline std::vector<physics::BoxShape> changedShapes;

	/// @brief Box shapes added this session but not yet attached to the body via Save.
	inline std::vector<physics::BoxShape> newShapes;
}