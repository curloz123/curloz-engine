/**
 * @file inspector.cpp
 * @author curl0z
 * @brief Inspector window's implementation
 */

#include "renderer/editor/inspector.hpp"
#include <imgui.h>
#include "renderer/editor/playertable.hpp"
#include "ecs/entitymanager.hpp"
#include "ecs/components.hpp"
#include "math/quateulerconv.hpp"
#include "renderer/editor/editor_types.hpp"
#include "renderer/assets/modeldata.hpp"

namespace clz::editor
{
	/// @brief Renders the Transform component section in the inspector.
	void showTransformComponentHeader();

	/// @brief Renders the Model component section in the inspector, if present.
	void showModelComponentHeader();
}

namespace clz::editor
{
	void showInspector()
	{
		/// Inspector Window
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

	}

	void showTransformComponentHeader()
	{
		if (!ImGui::CollapsingHeader("Transform"))
		{
			return;
		}

		auto& transform = ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());
		auto& euler = ecs::getComponent<ecs::EulerRotationComponent>(currentSelectedEntity.value());

		math::vec3 pos = transform.position;
		math::vec3 rot = euler.rotation;
		math::vec3 scale = transform.scale;

		ImGui::PushFont(fontMono);
		ImGui::SliderFloat3("Position", &pos.x, -100.0f, 100.0f);
		ImGui::SliderFloat3("Rotation", &rot.x, -180.0f, 180.0f);
		ImGui::SliderFloat3("Scale", &scale.x, 0.01f, 10.0f);
		ImGui::PopFont();

		euler.rotation = rot;
		transform.rotation = math::quatFromEuler(math::radians(rot));
		transform.position = pos;
		transform.scale = scale;
	}

	void showModelComponentHeader()
	{
		if (!ecs::hasComponent<ecs::ModelComponent>(currentSelectedEntity.value()))
		{
			return;
		}
		if (!ImGui::CollapsingHeader("Model"))
		{
			return;
		}

		const renderer::ModelID id = ecs::getComponent<ecs::ModelComponent>(
						currentSelectedEntity.value()).modelID;
		const auto name = renderer::Asset::getModelName(id);

		ImGui::PushFont(fontMono);
		ImGui::Text("Model Path: %s", name.c_str());
		ImGui::Separator();
		ImGui::PopFont();


	}
}