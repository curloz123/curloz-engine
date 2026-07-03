/**
 * @file playertable.cpp
 * @author curl0z
 * @brief Entities window's implementation
 */

#include "renderer/editor/playertable.hpp"
#include "renderer/editor/editor_types.hpp"
#include "scene/entity/entitymanager.hpp"
#include <imgui.h>

namespace clz::editor
{
	void showEntityTab()
	{
		ImGui::PushFont(fontMonoBold);
		if (!ImGui::BeginTabItem("Entities"))
		{
			ImGui::PopFont();
			return;
		}
		ImGui::PopFont();
		ImGui::PushFont(fontSans);
		const auto& entities = ecs::getEntities();
		for (auto& entity : entities)
		{
			const auto name = ecs::getEntityName(entity);
			bool selected = (currentSelectedEntity == entity);
			if (ImGui::Selectable(name.c_str(), selected))
				currentSelectedEntity = entity;
		}
		ImGui::PopFont();
		ImGui::EndTabItem();
	}
} // namespace clz::editor