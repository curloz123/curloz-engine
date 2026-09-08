/**
 * @file rigidbodycomponent.hpp
 * @author curl0z
 * @brief Editor UI component for Rigid Bodies.
 * Displays rigid body data in the Inspector and manages the offscreen
 * rendering of physics shapes for visual debugging.
 */
#pragma once

#include "entity/entitymanager.hpp"
#include "physics/physics.hpp"
#include <optional>
#include <vulkan/vulkan.h>

namespace clz::editor
{
	inline std::optional<ecs::entity> capturedEntity;
	inline std::optional<physics::RigidBodyShapeId> lastEditedShape;
	/**
	 * @brief Draws the main "RigidBody" collapsible header in the Inspector.
	 * @details Populates the capturedBodyData snapshot when the user clicks "Add Shape"
	 * or opens the editor. Handles basic body properties like type, damping, and locks.
	 */
	void showRigidBodyHeader();

	/**
	 * @brief Executes a secondary Vulkan render pass for the physics shape preview.
	 * @details Renders the entity's model and the pending/committed physics shapes
	 * into an offscreen target. This image is later sampled by ImGui to display
	 * a live 3D preview of the colliders inside the editor window.
	 * @param commandBuffer Active Vulkan command buffer, must be in recording state.
	 */
	void drawBodyEditorOffscreenImage(VkCommandBuffer commandBuffer);

	/**
	 * @brief Draws the "Shape Controls" and "RigidBody Shape Editor" ImGui windows.
	 * @details Handles the UI for tweaking individual shape properties (density,
	 * friction, dimensions) and manages the add/delete shape workflow.
	 */
	void presentBodyEditorWindow();

} // namespace clz::editor
