/**
 * @file inspector.cpp
 * @author curl0z
 * @brief Inspector window's implementation.
 *
 * Each editable field follows the same undo/redo capture pattern:
 * IsItemActivated() captures the pre-edit component state, and
 * IsItemDeactivatedAfterEdit() commits a snapshot once the edit is
 * confirmed complete (covers mouse drag, keyboard entry, tab-away — any
 * input method ImGui recognizes as "editing finished").
 */

#include "renderer/editor/inspector.hpp"
#include "math/angle.hpp"
#include "math/quateulerconv.hpp"
#include "renderer/assets/modeldata.hpp"
#include "renderer/editor/editor_types.hpp"
#include "renderer/editor/timemachine.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"
#include <imgui.h>

namespace clz::editor
{
	/// @brief Renders the Transform component section in the inspector.
	void showTransformComponentHeader();

	/// @brief Component state captured at the start of the current edit, used as the undo "before" value.
	ecs::TransformComponent previousTransform;

	/// @brief Renders the Model component section in the inspector, if present.
	void showModelComponentHeader();
} // namespace clz::editor

namespace clz::editor
{
	/// @brief Draws the Inspector window for the currently selected entity, and polls undo/redo.
	void showInspector()
	{
		ImGui::Begin("Inspector");
		if (currentSelectedEntity.has_value())
		{
			ImGui::PushFont(fontMonoBold);
			ImGui::Text("Selected Entity: %s", ecs::getEntityName(currentSelectedEntity.value()).c_str());
			ImGui::Separator();
			ImGui::PopFont();

			showTransformComponentHeader();
			ImGui::Separator();
			showModelComponentHeader();
		}
		else
		{
			ImGui::TextDisabled("No entity selected");
		}
		ImGui::End();

		// Check if undo or redo has to be performed
		timeTravel();
	}

	/// @brief Draws Position/Rotation/Scale sliders and records a snapshot when an edit completes.
	void showTransformComponentHeader()
	{
		if (!ImGui::CollapsingHeader("Transform"))
			return;

		auto transform = ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());
		auto euler = ecs::getComponent<ecs::EulerRotationComponent>(currentSelectedEntity.value());

		bool anyEditFinished = false;

		ImGui::PushFont(fontMono);

		ImGui::SliderFloat3("Position", &transform.position.x, -100.0f, 100.0f);
		if (ImGui::IsItemActivated())
		{
			ActiveTransform = TransformType::TRANSLATE;
			previousTransform = ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
			anyEditFinished = true;

		ImGui::SliderFloat3("Rotation", &euler.rotation.x, -180.0f, 180.0f);
		if (ImGui::IsItemActivated())
		{
			ActiveTransform = TransformType::ROTATE;
			previousTransform = ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
			anyEditFinished = true;

		ImGui::SliderFloat3("Scale", &transform.scale.x, 0.01f, 10.0f);
		if (ImGui::IsItemActivated())
		{
			ActiveTransform = TransformType::SCALE;
			previousTransform = ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
			anyEditFinished = true;

		ImGui::PopFont();

		// Live write-back every frame, for immediate visual feedback while editing
		ecs::TransformComponent newTransform;
		newTransform.position = transform.position;
		newTransform.rotation = math::quatFromEuler(math::radians(euler.rotation));
		newTransform.scale = transform.scale;
		ecs::setComponent<ecs::TransformComponent>(currentSelectedEntity.value(), newTransform);
		ecs::setComponent<ecs::EulerRotationComponent>(currentSelectedEntity.value(), euler);

		// Commit exactly one snapshot per completed edit, regardless of input method
		if (anyEditFinished)
		{
			createSnapshot<ecs::TransformComponent>(currentSelectedEntity.value(), previousTransform, newTransform);
		}
	}

	/// @brief Displays read-only Model component info, if the entity has one.
	void showModelComponentHeader()
	{
		if (!ecs::hasComponent<ecs::ModelComponent>(currentSelectedEntity.value()))
			return;
		if (!ImGui::CollapsingHeader("Model"))
			return;

		const renderer::ModelID id = ecs::getComponent<ecs::ModelComponent>(currentSelectedEntity.value()).modelID;
		const auto name = renderer::Asset::getModelName(id);

		ImGui::PushFont(fontMono);
		ImGui::Text("Model Path: %s", name.c_str());
		ImGui::Separator();
		ImGui::PopFont();
	}
} // namespace clz::editor