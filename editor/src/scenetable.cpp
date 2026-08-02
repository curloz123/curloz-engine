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

		ImGui::PushFont(fontSans);

		const auto& name = ecs::getEntityName(entity);
		const bool selected = (currentSelectedEntity == entity);

		const float iconSize = ImGui::GetFrameHeight();
		const float spacing = ImGui::GetStyle().ItemSpacing.x;

		const float selectableWidth =
			ImGui::GetContentRegionAvail().x - iconSize - spacing;

		if (ImGui::Selectable(name.c_str(), selected, 0, ImVec2(selectableWidth, 0)))
		{
			currentSelectedEntity = entity;
		}
		ImGui::PopFont();
		ImGui::SameLine();

		const ImVec2 p = ImGui::GetCursorScreenPos();

		ImGui::InvisibleButton(
			("##icon" + ecs::getEntityName(entity)).c_str(),
			ImVec2(iconSize, iconSize)
		);

		std::string icon = "NULL ICON";
		if (!ecs::isEntityDisabled(entity))
		{
			icon = "";

			if (ImGui::IsItemClicked())
				ecs::disableEntity(entity);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Disable entity");
		}
		else
		{
			icon = "";

			if (ImGui::IsItemClicked())
				ecs::enableEntity(entity);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Enable entity");
		}

		// Draw the icon centered.

		ImGui::PushFont(fontMono);

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		const ImVec2 textSize = ImGui::CalcTextSize(icon.c_str());

		drawList->AddText(
			fontMono,
			18,
			ImVec2(p.x + (iconSize - textSize.x) * 0.5f,
			       p.y + (iconSize - textSize.y) * 0.5f),
			ImGui::GetColorU32(ImGuiCol_Text),
			icon.c_str()
		);

		ImGui::PopFont();
	}
	ImGui::EndTabItem();
}
} // namespace clz::editor