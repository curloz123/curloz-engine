/**
* @file modelcomponent.cpp
 * @author curl0z
 * @brief displays model component's data n all
 * in inspector
 */

#include "../../include/inspector/modelcomponent.hpp"
#include "../../include/editor_types.hpp"
#include <imgui.h>
#include "entity/components.hpp"
#include "entity/componentmanager.hpp"
#include "renderer/model/model.hpp"
#include "../../include/scenetable.hpp"

namespace clz::editor
{
	/// @brief Displays read-only Model component info, if the entity has one.
	void showModelComponentHeader()
	{
		if (!ecs::hasComponent<ecs::ModelComponent>(currentSelectedEntity.value()))
			return;
		if (!ImGui::CollapsingHeader("Model"))
			return;

		const renderer::ModelId id = ecs::getComponent<ecs::ModelComponent>(currentSelectedEntity.value()).modelId;
		const auto name = renderer::getModelPath(id);

		ImGui::PushFont(fontMono);
		ImGui::Text("Model Path: %s", name.c_str());
		ImGui::Separator();
		ImGui::PopFont();
	}

}