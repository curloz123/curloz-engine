/**
 * @file transformcomponent.cpp
 * @author curl0z
 * @brief displays model component's data n all
 * in inspector. Internally can change rigid body's
 * data and body's attributes.
 */

#include "../../include/inspector/transformcomponent.hpp"
#include "../../include/inspector/inspector.hpp"
#include "../../include/editor_types.hpp"
#include "../../include/timemachine.hpp"
#include <imgui.h>
#include "math/quateulerconv.hpp"
#include "math/angle.hpp"
#include "scene/entity/components.hpp"
#include "scene/entity/componentmanager.hpp"

namespace clz::editor
{
	/// @brief Component state captured at the start of the current edit, used as the undo "before" value.
	 ecs::EditorTransformComponent previousTransform;

	/// @brief Draws Position/Rotation/Scale sliders and records a snapshot when an edit completes.
	void showTransformComponentHeader()
	{
		if (!ImGui::CollapsingHeader("Transform"))
			return;

		auto& editorTransform = ecs::getComponent<ecs::EditorTransformComponent>(currentSelectedEntity.value());
		auto& euler = ecs::getComponent<ecs::EulerRotationComponent>(currentSelectedEntity.value());

		bool anyEditFinished = false;

		ImGui::PushFont(fontMono);

		ImGui::SliderFloat3("Position", &editorTransform.position.x, -100.0f, 100.0f);
		if (ImGui::IsItemActivated())
		{
			ActiveTransform = TransformType::TRANSLATE;
			previousTransform = ecs::getComponent<ecs::EditorTransformComponent>(currentSelectedEntity.value());
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
			anyEditFinished = true;

		ImGui::SliderFloat3("Rotation", &euler.rotation.x, -179.9f, 179.9f);
		if (ImGui::IsItemActivated())
		{
			ActiveTransform = TransformType::ROTATE;
			previousTransform = ecs::getComponent<ecs::EditorTransformComponent>(currentSelectedEntity.value());
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
			anyEditFinished = true;

		ImGui::SliderFloat3("Scale", &editorTransform.scale.x, 0.01f, 10.0f);
		if (ImGui::IsItemActivated())
		{
			ActiveTransform = TransformType::SCALE;
			previousTransform = ecs::getComponent<ecs::EditorTransformComponent>(currentSelectedEntity.value());
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
			anyEditFinished = true;

		ImGui::PopFont();

		// Writing back data
		editorTransform.rotation = math::quatFromEuler(math::radians(euler.rotation));

		auto& transform = ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());
		transform.position = editorTransform.position;
		transform.rotation = editorTransform.rotation;
		transform.scale = editorTransform.scale;

		// Commit exactly one snapshot per completed edit, regardless of input method
		if (anyEditFinished)
		{
			createSnapshot<ecs::EditorTransformComponent>(currentSelectedEntity.value(),
				previousTransform, editorTransform);
		}
	}

}