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

#include "../../include/inspector/inspector.hpp"
#include "../../include/editor_types.hpp"
#include "../../include/inspector/modelcomponent.hpp"
#include "../../include/inspector/transformcomponent.hpp"
#include "../../include/scenetable.hpp"
#include "../../include/timemachine.hpp"
#include "entity/componentmanager.hpp"
#include "include/inspector/rigidbodycomponent.hpp"
#include "physics/physicscomponent.hpp"
#include "renderer/rendercomponent.hpp"
#include <imgui.h>

namespace clz::editor
{
void showComponentSpecificWindows();
}
namespace clz::editor
{
/// @brief Draws the Inspector window for the currently selected entity, and polls undo/redo.
void showInspector(VkCommandBuffer commandBuffer)
{
	if (!ImGui::Begin("Inspector"))
	{
		ImGui::End();
		return;
	}

	if (currentSelectedEntity.has_value())
	{
		ImGui::PushFont(fontMonoBold, 20);
		ImGui::Text(
			"Selected Entity: %s",
			ecs::getEntityName(currentSelectedEntity.value()).c_str()
		);
		ImGui::PopFont();

		ImGui::Separator();

		ImGui::PushFont(fontMono, 18);
		ImGui::Text("Entity is currently: ");

		ImGui::SameLine();
		if (!ecs::isEntityDisabled(currentSelectedEntity.value()))
		{
			ImGui::Text("Enabled");
		}
		else
		{
			ImGui::Text("Disabled");
		}
		ImGui::PopFont();

		ImGui::Separator();

		// Every entity has transform component
		showTransformComponentHeader();
		ImGui::Separator();

		if (ecs::hasComponent<renderer::ModelComponent>(
			    currentSelectedEntity.value()
		    ))
			showModelComponentHeader();
		ImGui::Separator();

		if (ecs::hasComponent<physics::RigidBodyComponent>(
			    currentSelectedEntity.value()
		    ))
			showRigidBodyHeader();
		ImGui::Separator();
	}
	else
	{
		ImGui::TextDisabled("No entity selected");
	}

	ImGui::End();

	showComponentSpecificWindows();
}

void showComponentSpecificWindows()
{
}
} // namespace clz::editor