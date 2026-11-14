/**
 * @file rigidbodycomponent.hpp
 * @author curl0z
 * @brief Editor UI component for Rigid Bodies.
 * Displays rigid body data in the Inspector and manages the offscreen
 * rendering of physics shapes for visual debugging.
 */
#pragma once

#include "entity/entitymanager.hpp"
#include "include/offscreen/offscreentarget.hpp"
#include "physics/physics.hpp"
#include <optional>
#include <vulkan/vulkan.h>

namespace clz::editor
{
	inline std::optional<ecs::entity> capturedEntity;
	inline std::optional<physics::RigidBodyShapeId> lastEditedShape;

	/// Body Editor offscreen image
	inline OffscreenImage bodyEditorImage;

	/**
	 * @brief Draws the main "RigidBody" collapsible header in the Inspector.
	 * @details Populates the capturedBodyData snapshot when the user clicks "Add Shape"
	 * or opens the editor. Handles basic body properties like type, damping, and locks.
	 * @param commandBuffer Active command buffer in-recording.
	 */
	void showRigidBodyHeader(VkCommandBuffer commandBuffer);

	void drawBodyScene(VkCommandBuffer commandBuffer);

	void showRigidBodySpecificWindows();
} // namespace clz::editor
