/**
 * @file transformcomponent.cpp
 * @author curl0z
 * @brief displays model component's data n all
 * in inspector. Internally can change rigid body's
 * data and body's attributes.
 */

#include "../../include/inspector/transformcomponent.hpp"
#include "../../include/editor_types.hpp"
#include "../../include/inspector/inspector.hpp"
#include "../../include/scenetable.hpp"
#include "../../include/timemachine.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "math/angle.hpp"
#include "math/quateulerconv.hpp"
#include <imgui.h>

namespace clz::editor
{
	/// @brief Component state captured at the start of the current edit, used as the undo
	/// "before" value.
	ecs::EditorTransformComponent previousEditorTransform;
	ecs::TransformComponent previousTransform;


	/// @brief Draws Position/Rotation/Scale sliders and records a snapshot when an edit
	/// completes.
	void showTransformComponentHeader()
	{
		if (!ImGui::CollapsingHeader("Transform"))
			return;

		auto& editorTransform = ecs::getComponent<ecs::EditorTransformComponent>(
			currentSelectedEntity.value()
		);

		bool anyChangeStarted = false;
		bool anyEditFinished = false;

		ImGui::PushFont(fontMono);

		auto saveOldData = [](){
			previousEditorTransform = ecs::getComponent<ecs::EditorTransformComponent>(
				currentSelectedEntity.value()
			);
			previousTransform = ecs::getComponent<ecs::TransformComponent>(
					currentSelectedEntity.value()
			);
		};

		if (ImGui::SliderFloat3("Position", &editorTransform.position.x, -100.0f, 100.0f))
		{
			ActiveTransform = TransformType::TRANSLATE;
			saveOldData();
			anyChangeStarted = true;
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			anyEditFinished = true;
		}

		if (ImGui::SliderFloat3("Rotation", &editorTransform.rotation.x, -179.9f, 179.9f))
		{
			ActiveTransform = TransformType::ROTATE;
			saveOldData();
			anyChangeStarted = true;
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			anyEditFinished = true;
		}

		if (ImGui::SliderFloat3("Scale", &editorTransform.scale.x, 0.01f, 10.0f))
		{
			ActiveTransform = TransformType::SCALE;
			saveOldData();
			anyChangeStarted = true;
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			anyEditFinished = true;
		}

		ImGui::PopFont();

		if (anyChangeStarted)
		{
			ecs::setComponent<ecs::TransformComponent>(currentSelectedEntity.value(), ecs::TransformComponent(
			math::quatFromEuler(math::radians(editorTransform.rotation)),
				editorTransform.position,
				editorTransform.scale)
			);
		}

		// Commit exactly one snapshot per completed edit, regardless of input method
		if (anyEditFinished)
		{
			ecs::TransformComponent newTransform(
				math::quatFromEuler(math::radians(editorTransform.rotation)),
				editorTransform.position,
				editorTransform.scale
			);
			auto oldTransform = previousTransform;
			ecs::EditorTransformComponent newEditorTransform = editorTransform;
			ecs::EditorTransformComponent oldEditorTransform = previousEditorTransform;

			const auto entityId = currentSelectedEntity.value();

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

	}

} // namespace clz::editor
