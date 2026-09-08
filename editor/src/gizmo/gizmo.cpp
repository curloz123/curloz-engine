#include "../../include/gizmo/gizmo.hpp"
#include <imgui.h>
#include <ImGuizmo.h>
#include "math/angle.hpp"
#include "math/quateulerconv.hpp"
#include "math/worldtransform.hpp"
#include <array>
#include "core/logs.hpp"

namespace clz::editor
{
	/// @copydoc gizmoTransform
	std::tuple<ecs::TransformComponent, ecs::EditorTransformComponent, bool> gizmoTransform(
		const Rect2D& viewport,
		clz::math::mat4& projection,
		const clz::math::mat4& view,
		const ecs::TransformComponent transform, 
		const bool uniformScale
	)
	{
		ImGuizmo::BeginFrame();

		projection.data[5] *= -1; // Flip Y to match ImGuizmo's coordinate system.

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::SetRect(
			(float)viewport.x,
			(float)viewport.y,
			(float)viewport.width,
			(float)viewport.height
		);

		ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		switch (ActiveTransform)
		{
		case (TransformType::TRANSLATE):
			operation = ImGuizmo::TRANSLATE;
			break;
		case (TransformType::SCALE):
			operation = ImGuizmo::SCALE;
			if (uniformScale)
				operation = ImGuizmo::SCALEU;
			break;
		case (TransformType::ROTATE):
			operation = ImGuizmo::ROTATE;
			break;
		}

		ecs::TransformComponent transformComponent = transform;
		ecs::EditorTransformComponent eTransformComponent(transform);
		math::mat4 model = math::getModelMatrix(
				transform.rotation,
				transform.position,
				transform.scale
		);

		ImGuizmo::Manipulate(
			    view.data,
			    projection.data,
			    operation,
			    ImGuizmo::LOCAL,
			    model.data
		);
		/// Check if gizmo is even held down by the mouse
		bool operationHappenedThisFrame = ImGuizmo::IsUsing();

		if (operationHappenedThisFrame)
		{
			std::array<float, 3> pos;			
			std::array<float, 3> rot;			
			std::array<float, 3> scale;			
			ImGuizmo::DecomposeMatrixToComponents(
					model.data, 
					pos.data(), rot.data(), scale.data());

			transformComponent.position = math::vec3(pos[0], pos[1], pos[2]);
			transformComponent.rotation = math::quatFromEuler(
					math::radians(math::vec3(rot[0], rot[1], rot[2])));
			transformComponent.scale    = math::vec3(scale[0], scale[1], scale[2]);

			eTransformComponent.position = math::vec3(pos[0], pos[1], pos[2]);
			eTransformComponent.rotation = math::vec3(rot[0], rot[1], rot[2]);
			eTransformComponent.scale    = math::vec3(scale[0], scale[1], scale[2]);
		}



		return std::make_tuple(
				transformComponent, 
				eTransformComponent, 
				operationHappenedThisFrame);
	}

	/// @copydoc gizmoTransform
	std::tuple<ecs::TransformComponent, ecs::EditorTransformComponent, bool> gizmoTransform(
		const Rect2D& viewport,
		clz::math::mat4& projection,
		const clz::math::mat4& view,
		const ecs::EditorTransformComponent eTransform, 
		const bool uniformScale
	)
	{
		ImGuizmo::BeginFrame();

		projection.data[5] *= -1; // Flip Y to match ImGuizmo's coordinate system.

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::SetRect(
			(float)viewport.x,
			(float)viewport.y,
			(float)viewport.width,
			(float)viewport.height
		);

		ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		switch (ActiveTransform)
		{
		case (TransformType::TRANSLATE):
			operation = ImGuizmo::TRANSLATE;
			break;
		case (TransformType::SCALE):
			operation = ImGuizmo::SCALE;
			if (uniformScale)
				operation = ImGuizmo::SCALEU;
			break;
		case (TransformType::ROTATE):
			operation = ImGuizmo::ROTATE;
			break;
		}

		ecs::TransformComponent transformComponent;
		transformComponent.position = eTransform.position;
		transformComponent.rotation = math::quatFromEuler(math::radians(eTransform.rotation));
		transformComponent.scale = eTransform.scale;
		ecs::EditorTransformComponent eTransformComponent = eTransform;

		math::mat4 model = math::getModelMatrix(
				transformComponent.rotation,
				transformComponent.position,
				transformComponent.scale
		);
		ImGuizmo::Manipulate(
			    view.data,
			    projection.data,
			    operation,
			    ImGuizmo::LOCAL,
			    model.data
		);


		bool operationHappenedThisFrame = ImGuizmo::IsUsing();
		if (operationHappenedThisFrame)
		{
			operationHappenedThisFrame = true;

			std::array<float, 3> pos;			
			std::array<float, 3> rot;			
			std::array<float, 3> scale;			
			ImGuizmo::DecomposeMatrixToComponents(
					model.data, 
					pos.data(), rot.data(), scale.data());



			transformComponent.position = math::vec3(pos[0], pos[1], pos[2]);
			transformComponent.rotation = math::quatFromEuler(
					math::radians(math::vec3(rot[0], rot[1], rot[2])));
			transformComponent.scale    = math::vec3(scale[0], scale[1], scale[2]);

			eTransformComponent.position = math::vec3(pos[0], pos[1], pos[2]);
			eTransformComponent.rotation = math::vec3(rot[0], rot[1], rot[2]);
			eTransformComponent.scale    = math::vec3(scale[0], scale[1], scale[2]);
		}

		return std::make_tuple(transformComponent, eTransformComponent, operationHappenedThisFrame);
	}
}
