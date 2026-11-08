/**
 * @file transformcomponent.cpp
 * @author curl0z
 * @brief displays model component's data n all
 * in inspector. Internally can change rigid body's
 * data and body's attributes.
 */

#include "../../include/inspector/transformcomponent.hpp"
#include "../../include/editor_types.hpp"
#include "../../include/scenetable.hpp"
#include "../../include/timemachine.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "math/angle.hpp"
#include "math/quateulerconv.hpp"
#include <imgui.h>
#include "../../include/gizmo/gizmo.hpp"

namespace clz::editor
{
	/// @brief Draws Position/Rotation/Scale sliders and records a snapshot when an edit
	/// completes.
	void showTransformComponentHeader()
	{
		if (!ImGui::CollapsingHeader("󰵉 Transform"))
			return;

		static ecs::EditorTransformComponent previousEditorTransform;
		static ecs::TransformComponent previousTransform;

		auto editorTransform = ecs::getComponent<ecs::EditorTransformComponent>(
			currentSelectedEntity.value()
		);

		bool anyChange = false;
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

		auto pos = editorTransform.getPosition();
		if (ImGui::SliderFloat3("Position", &pos.x, -100.0f, 100.0f))
		{
			editorTransform.setPosition(pos);
			anyChange = true;
		}
		if (ImGui::IsItemActivated())
		{
			ActiveTransform = TransformType::TRANSLATE;
			saveOldData();
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			anyEditFinished = true;
		}

		auto rot = editorTransform.getRotation();
		if (ImGui::SliderFloat3("Rotation", &rot.x, -179.9f, 179.9f))
		{
			editorTransform.setRotation(rot);
			anyChange = true;
		}
		if (ImGui::IsItemActivated())
		{
			ActiveTransform = TransformType::ROTATE;
			saveOldData();
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			anyEditFinished = true;
		}

		auto scale = editorTransform.getScale();
		if (ImGui::SliderFloat3("Scale", &scale.x, 0.01f, 10.0f))
		{
			editorTransform.setScale(scale);
			anyChange = true;
		}
		if (ImGui::IsItemActivated())
		{
			ActiveTransform = TransformType::SCALE;
			saveOldData();
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			anyEditFinished = true;
		}

		ImGui::PopFont();

		if (anyChange)
		{
			ecs::setComponent<ecs::EditorTransformComponent>(currentSelectedEntity.value(), editorTransform);
			ecs::setComponent<ecs::TransformComponent>(
				currentSelectedEntity.value(),
				ecs::TransformComponent(
					math::quatFromEuler(math::radians(editorTransform.getRotation())),
					editorTransform.getPosition(),
					editorTransform.getScale()
				)
			);
		}

		// Commit exactly one snapshot per completed edit, regardless of input method
		if (anyEditFinished)
		{
			ecs::TransformComponent newTransform(
				math::quatFromEuler(math::radians(editorTransform.getPosition())),
				editorTransform.getPosition(),
				editorTransform.getPosition()
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

		}

	}
} // namespace clz::editor
