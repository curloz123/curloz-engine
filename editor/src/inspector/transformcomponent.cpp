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
static ecs::EditorTransformComponent previousTransform;

/// @brief Draws Position/Rotation/Scale sliders and records a snapshot when an edit
/// completes.
void showTransformComponentHeader()
{
	if (!ImGui::CollapsingHeader("Transform"))
		return;

	auto& editorTransform = ecs::getComponent<ecs::EditorTransformComponent>(
		currentSelectedEntity.value()
	);

	bool anyEditFinished = false;

	ImGui::PushFont(fontMono);

	ImGui::SliderFloat3("Position", &editorTransform.position.x, -100.0f, 100.0f);
	if (ImGui::IsItemActivated())
	{
		ActiveTransform = TransformType::TRANSLATE;
		previousTransform = ecs::getComponent<ecs::EditorTransformComponent>(
			currentSelectedEntity.value()
		);
	}
	if (ImGui::IsItemDeactivatedAfterEdit())
		anyEditFinished = true;

	ImGui::SliderFloat3("Rotation", &editorTransform.rotation.x, -179.9f, 179.9f);
	if (ImGui::IsItemActivated())
	{
		ActiveTransform = TransformType::ROTATE;
		previousTransform = ecs::getComponent<ecs::EditorTransformComponent>(
			currentSelectedEntity.value()
		);
	}
	if (ImGui::IsItemDeactivatedAfterEdit())
		anyEditFinished = true;

	ImGui::SliderFloat3("Scale", &editorTransform.scale.x, 0.01f, 10.0f);
	if (ImGui::IsItemActivated())
	{
		ActiveTransform = TransformType::SCALE;
		previousTransform = ecs::getComponent<ecs::EditorTransformComponent>(
			currentSelectedEntity.value()
		);
	}
	if (ImGui::IsItemDeactivatedAfterEdit())
		anyEditFinished = true;

	ImGui::PopFont();

	ecs::TransformComponent newTransform(
		math::quatFromEuler(math::radians(editorTransform.rotation)),
		editorTransform.position,
		editorTransform.scale
	);

	// Commit exactly one snapshot per completed edit, regardless of input method
	if (anyEditFinished)
	{
		const auto entityId = currentSelectedEntity.value();

		auto oldTransform = ecs::TransformComponent(
			math::quatFromEuler(math::radians(previousTransform.rotation)),
			previousTransform.position,
			previousTransform.scale
		);
		timemachine::createSnapshot(
			[entityId, oldTransform] {
				ecs::setComponent<ecs::TransformComponent>(
					entityId,
					oldTransform
				);
				ecs::setComponent<ecs::EditorTransformComponent>(
					entityId,
					ecs::EditorTransformComponent(oldTransform)
				);
			},
			[entityId, newTransform] {
				ecs::setComponent<ecs::TransformComponent>(
					entityId,
					newTransform
				);
				ecs::setComponent<ecs::EditorTransformComponent>(
					entityId,
					ecs::EditorTransformComponent(newTransform)
				);
			}
		);

		clz::log::debug("created snapshot");
	}

	// Writing back data
	ecs::setComponent<ecs::TransformComponent>(
		currentSelectedEntity.value(),
		newTransform
	);
}

} // namespace clz::editor