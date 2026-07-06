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
#include "../../include/timemachine.hpp"
#include "../../include/inspector/transformcomponent.hpp"
#include "../../include/inspector/modelcomponent.hpp"
#include "renderer/assets/modeldata.hpp"
#include <imgui.h>
#include "include/inspector/rigidbodycomponent.hpp"

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
			ImGui::Separator();
			showRigidBodyHeader();
		}
		else
		{
			ImGui::TextDisabled("No entity selected");
		}
		ImGui::End();

		// Check if undo or redo has to be performed
		timeTravel();
	}
} // namespace clz::editor