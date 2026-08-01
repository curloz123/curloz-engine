/**
@file rigidbodycomponent.hpp
@author curl0z
@brief Editor UI component for Rigid Bodies.
Displays rigid body data in the Inspector and manages the offscreen
rendering of physics shapes for visual debugging.
*/
#pragma once
#include "entity/entitymanager.hpp"
#include "physics/body.hpp"
#include "physics/shape.hpp"
#include "renderer/shapes.hpp"
#include <optional>

// Forward declaration for Vulkan command buffer
struct VkCommandBuffer_T;
typedef struct VkCommandBuffer_T* VkCommandBuffer;

namespace clz::editor
{
/**
 * @brief Captures a snapshot of a rigid body's state for editor manipulation.
 * @details This struct acts as a "pending state" buffer. By copying the live
 * physics data into this struct, the ImGui sliders can be manipulated without
 * immediately waking up or recalculating the live physics simulation. Changes
 * are only committed to the physics engine when the user clicks "Save".
 */
struct EditorBodyData
{
	std::optional<ecs::entity> entityId; ///< @brief The entity currently being edited.
	physics::BodyType type; ///< @brief Pending body type (Static, Dynamic, Kinematic).
	bool enableSleep;	///< @brief Pending sleep state.
	math::vec3 position;	///< @brief Pending world position.
	math::quat rotation;	///< @brief Pending world rotation.
	float linearDamping;	///< @brief Pending linear damping.
	float angularDamping;	///< @brief Pending angular damping.
	std::array<bool, 3> linearLocks;  ///< @brief Pending linear axis locks.
	std::array<bool, 3> angularLocks; ///< @brief Pending angular axis locks.

	/// @brief Pending shape definitions. Separated from live shapes to allow safe UI
	/// tweaking.
	std::vector<physics::ShapeDef> shapeData;

	/// @brief Clears the captured data, effectively closing the editor session.
	void clearData()
	{
		entityId.reset();
		shapeData.clear();
	}
};

/// @brief Global state holding the currently edited body's pending changes.
/// @note This is a singleton-style global because only one entity's rigid body
/// can be edited in the Inspector at a time.
inline EditorBodyData capturedBodyData;

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