/**
 * @file gizmo.cpp
 * @author curl0z
 * @brief Implementation of transform gizmo
 */
#include "renderer/editor/gizmo.hpp"
#include "math/angle.hpp"
#include "math/quateulerconv.hpp"
#include "math/worldtransform.hpp"
#include "renderer/camera/cameramatrices.hpp"
#include "renderer/editor/editor_types.hpp"
#include "renderer/editor/inspector.hpp"
#include "renderer/vk_types.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"
#include <ImGuizmo.h>
#include <imgui.h>

namespace clz::editor
{
	/// Draw gizmo function
	void drawGizmo()
	{
		ImGuizmo::BeginFrame();

		// return if no entity has been selected yet
		if (!currentSelectedEntity.has_value())
		{
			return;
		}

		// retrieve entity's transform and euler rotation component
		auto& transform = ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());
		auto& eulerRot = ecs::getComponent<ecs::EulerRotationComponent>(currentSelectedEntity.value());

		// retrieve matrices
		math::mat4 model = math::getModelMatrix(transform.rotation, transform.position, transform.scale);
		const math::mat4 view = renderer::camera::getViewMatrix();
		math::mat4 proj = renderer::camera::getProjectionMatrix();
		// Multiply -1 because gizmo expects OpenGL style matrix
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

		// decompose mat back into position/rotation/scale
		float pos[3], rot[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.data, pos, rot, scale);
		eulerRot.rotation = math::vec3(rot[0], rot[1], rot[2]);

		transform.position = math::vec3(pos[0], pos[1], pos[2]);
		transform.rotation = math::quatFromEuler(math::radians(eulerRot.rotation));
		transform.scale = math::vec3(scale[0], scale[1], scale[2]);
	}
} // namespace clz::editor