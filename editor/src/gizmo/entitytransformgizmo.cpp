/**
 * @file entitytransformgizmo.cpp
 * @brief Implementation of the transform gizmo for editing entity transforms.
 *
 * Integrates ImGuizmo to provide interactive translation, rotation, and scaling
 * of the currently selected entity in the 3D viewport.
 */

#include "../../include/gizmo/gizmo.hpp"
#include "../../include/inspector/inspector.hpp"
#include "../../include/scenetable.hpp"
#include "../../include/timemachine.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "include/sceneview.hpp"
#include "math/angle.hpp"
#include "math/quateulerconv.hpp"
#include "math/worldtransform.hpp"
#include "renderer/camera/camera.hpp"
#include <imgui.h>
#include <ImGuizmo.h>

namespace clz::editor
{
	/// @brief Edge-detection state for ImGuizmo::IsUsingAny(), since it's level-triggered.
	bool gizmoUsedLastFrame = false;
	bool gizmoUsedThisFrame = false;

	/// @brief Transform captured at the start of the current gizmo drag, used as the undo
	/// "before" value.
	inline ecs::EditorTransformComponent previousGizmoEditorTransform;
	inline ecs::TransformComponent previousGizmoTransform;

	/**
	 * @brief Draws and handles the transform gizmo for the currently selected entity.
	 * @param viewport The screen rectangle (in pixels) where the gizmo is active.
	 *
	 * @par Decision: Editor vs. runtime transform separation
	 *      The runtime transform uses quaternions for interpolation, while the
	 *      editor uses Euler angles for intuitive editing. The gizmo manipulates
	 *      the editor copy, and on each change, the runtime quaternion is updated
	 *      from the Euler angles.
	 *
	 * @par Decision: Invert projection matrix Y axis
	 *      `proj.data[5] *= -1` is used because ImGuizmo expects a projection
	 *      matrix with Y pointing up, while the renderer uses the opposite
	 *      convention. This ensures the gizmo manipulator aligns correctly with
	 *      the 3D scene.
	 *
	 * @par Decision: Operation selection via global enum
	 *      The active operation (translate, rotate, scale) is determined by the
	 *      global `ActiveTransform` variable (likely set by the user interface,
	 *      e.g., via toolbar buttons).
	 *
	 * @par Decision: World space manipulation
	 *      The gizmo is set to `WORLD` space for consistency; local space might
	 *      be added later as an option.
	 */
	void drawEntityTransformGizmo(const Rect2D& viewport)
	{
		if (!currentSelectedEntity.has_value())
		{
			return;
		}

		ImGuizmo::BeginFrame();

		auto entityTransform =
			ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());
		auto editorTransform = ecs::getComponent<ecs::EditorTransformComponent>(
			currentSelectedEntity.value()
		);

		math::mat4 model = math::getModelMatrix(
			entityTransform.rotation,
			entityTransform.position,
			entityTransform.scale
		);

		const math::mat4 view = renderer::getCameraViewMatrix(mainViewportImage.cameraId);

		math::mat4 proj = renderer::getCameraProjMatrix(
			mainViewportImage.cameraId,
			static_cast<float>(mainViewportImage.extent.width),
			static_cast<float>(mainViewportImage.extent.height)
		);
		proj.data[5] *= -1; // Flip Y to match ImGuizmo's coordinate system.

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::SetRect(
			(float)viewport.x,
			(float)viewport.y,
			(float)viewport.width,
			(float)viewport.height
		);

		gizmoUsedThisFrame = ImGuizmo::IsUsingAny();
		if (gizmoUsedThisFrame && !gizmoUsedLastFrame)
		{
			// Drag just started — capture the pre-drag state
			previousGizmoEditorTransform = editorTransform;
			previousGizmoTransform = entityTransform;
		}

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
		if (ImGuizmo::Manipulate(
			    view.data,
			    proj.data,
			    operation,
			    ImGuizmo::LOCAL,
			    model.data
		    ))
		{
			float pos[3], rot[3], scale[3];
			ImGuizmo::DecomposeMatrixToComponents(model.data, pos, rot, scale);
			editorTransform.position = math::vec3(pos[0], pos[1], pos[2]);
			editorTransform.rotation = math::vec3(rot[0], rot[1], rot[2]);
			editorTransform.scale = math::vec3(scale[0], scale[1], scale[2]);

			ecs::setComponent(currentSelectedEntity.value(), editorTransform);
			entityTransform = ecs::TransformComponent(
					math::quatFromEuler(math::radians(editorTransform.rotation)),
					editorTransform.position,
					editorTransform.scale
			);
			ecs::setComponent(currentSelectedEntity.value(), entityTransform);
		}

		if (!gizmoUsedThisFrame && gizmoUsedLastFrame)
		{
			entityTransform.position = editorTransform.position;
			entityTransform.rotation =
				math::quatFromEuler(math::radians(editorTransform.rotation));
			entityTransform.scale = editorTransform.scale;

			const auto entityId = currentSelectedEntity.value();

			const auto oldTransform = previousGizmoTransform;
			const auto newTransform = entityTransform;
			const auto oldEditorTransform = previousGizmoEditorTransform;
			const auto newEditorTransform = editorTransform;
			timemachine::createSnapshot(
				[entityId, oldTransform, oldEditorTransform] {
					ecs::setComponent<ecs::TransformComponent>(
						entityId,
						oldTransform
					);
					ecs::setComponent<ecs::EditorTransformComponent>(
						entityId,
						oldEditorTransform
					);
				},
				[entityId, newTransform, newEditorTransform] {
					ecs::setComponent<ecs::TransformComponent>(
						entityId,
						newTransform
					);
					ecs::setComponent<ecs::EditorTransformComponent>(
						entityId,
						newEditorTransform
					);
				}
			);

			clz::log::debug("created snapshot");
		}
		gizmoUsedLastFrame = gizmoUsedThisFrame;
	}
} // namespace clz::editor
