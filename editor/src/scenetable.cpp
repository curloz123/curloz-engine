/**
 * @file scenetable.cpp
 * @author curl0z
 * @brief Scene window implementation
 *
 * Show's the scene's data like entities, lights etc...
 */

#include "../include/scenetable.hpp"
#include "../include/editor_types.hpp"
#include "entity/entitymanager.hpp"
#include <imgui.h>

namespace clz::editor
{
	/// @copydoc
	void showSceneTab()
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