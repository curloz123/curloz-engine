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
	const auto& entities = ecs::getEntities();
	for (auto& entity : entities)
	{
		if (ecs::isMarkedForDeletion(entity))
			continue;

		ImGui::PushID(ecs::getEntityName(entity).c_str());
		ImGui::PushFont(fontSans);

		const auto name = ecs::getEntityName(entity);
		bool selected = (currentSelectedEntity == entity);
		if (ImGui::Selectable(name.c_str(), selected))
			currentSelectedEntity = entity;

		ImGui::PopFont();

		ImGui::SameLine();

		if (!ecs::isEntityDisabled(entity))
		{
			ImGui::PushFont(fontMono);
			if (ImGui::Button("󰈈"))
			{
				ecs::disableEntity(entity);
			}
			ImGui::PopFont();

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Disable entity");
		}
		else
		{
			ImGui::PushFont(fontMono);
			if (ImGui::Button("󰈉"))
			{
				ecs::enableEntity(entity);
			}
			ImGui::PopFont();

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Enable entity");
		}
		ImGui::PopID();
	}
	ImGui::EndTabItem();
}
} // namespace clz::editor