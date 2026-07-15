/**
 * @file gizmo.cpp
 * @author curl0z
 * @brief Implementation of the transform gizmo.
 *
 * Undo/redo capture mirrors the inspector's pattern, but keyed off
 * ImGuizmo::IsUsingAny() instead of ImGui's item-activation events, since
 * IsUsingAny() is level-triggered (true for every frame of an active drag,
 * and also true on pure hover) rather than edge-triggered — so the
 * false->true / true->false transitions are tracked manually here.
 */
#include "../include/gizmo.hpp"
#include "../include/editor_types.hpp"
#include "../include/inspector/inspector.hpp"
#include "math/angle.hpp"
#include "math/quateulerconv.hpp"
#include "math/worldtransform.hpp"
#include "renderer/camera/cameramatrices.hpp"
#include "renderer/vk_types.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"
#include "../include/timemachine.hpp"
#include <ImGuizmo.h>
#include <imgui.h>

namespace clz::gizmo
{
	/// @brief Edge-detection state for ImGuizmo::IsUsingAny(), since it's level-triggered.
	bool gizmoUsedLastFrame = false;
	bool gizmoUsedThisFrame = false;

	/// @brief Transform captured at the start of the current gizmo drag, used as the undo "before" value.
	inline ecs::EditorTransformComponent previousTransform;
} // namespace clz::gizmo

namespace clz::editor
{
	/// @brief Draws and manipulates the transform gizmo for the currently selected entity.
	void drawGizmo()
	{
		ImGuizmo::BeginFrame();

		if (!currentSelectedEntity.has_value())
		{
			gizmo::gizmoUsedLastFrame = false;
			return;
		}

		auto& editorTransform = ecs::getComponent<ecs::EditorTransformComponent>(currentSelectedEntity.value());
		auto& eulerRot = ecs::getComponent<ecs::EulerRotationComponent>(currentSelectedEntity.value());

		gizmo::gizmoUsedThisFrame = ImGuizmo::IsUsingAny();
		if (gizmo::gizmoUsedThisFrame && !gizmo::gizmoUsedLastFrame)
		{
			// Drag just started — capture the pre-drag state
			gizmo::previousTransform = editorTransform;
		}

		math::mat4 model = math::getModelMatrix(editorTransform.rotation, editorTransform.position, editorTransform.scale);
		const math::mat4 view = renderer::camera::getViewMatrix();
		math::mat4 proj = renderer::camera::getProjectionMatrix();
		// Flip Y — ImGuizmo assumes an OpenGL-style projection, ours is Vulkan (Y-flipped)
		proj.data[5] *= -1;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
		ImGuizmo::SetRect(0, 0, renderer::r_swapchainContext.extent.width, renderer::r_swapchainContext.extent.height);

		ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		switch (ActiveTransform)
		{
		case (TransformType::TRANSLATE):
			operation = ImGuizmo::TRANSLATE;
			break;
		case (TransformType::SCALE):
			operation = ImGuizmo::SCALE;
			break;
		case (TransformType::ROTATE):
			operation = ImGuizmo::ROTATE;
			break;
		}

		ImGuizmo::Manipulate(view.data, proj.data, operation, ImGuizmo::LOCAL, model.data);

		// Decompose the manipulated matrix back into position/rotation/scale
		float pos[3], rot[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.data, pos, rot, scale);

		eulerRot.rotation = math::vec3(rot[0], rot[1], rot[2]);
		editorTransform.position = math::vec3(pos[0], pos[1], pos[2]);
		editorTransform.rotation = math::quatFromEuler(math::radians(eulerRot.rotation));
		editorTransform.scale = math::vec3(scale[0], scale[1], scale[2]);

		const auto transform = ecs::TransformComponent(
			editorTransform.rotation, editorTransform.position, editorTransform.scale);
		ecs::setComponent<ecs::TransformComponent>(currentSelectedEntity.value(), transform);

		if (!gizmo::gizmoUsedThisFrame && gizmo::gizmoUsedLastFrame)
		{
			// Drag just ended, create a snapshot
			createSnapshot<ecs::EditorTransformComponent>(currentSelectedEntity.value(), gizmo::previousTransform, editorTransform);
		}
		gizmo::gizmoUsedLastFrame = gizmo::gizmoUsedThisFrame;
	}
} // namespace clz::editor