/**
 * @file topbar.cpp
 * @brief Implementation of the editor's top menu bar and docking functions.
 *
 * Uses Dear ImGui to create the main menu bar with File/Edit menus and
 * window control buttons (minimize, maximize, close).
 */

#include "../include/topbar.hpp"
#include "../include/editor_types.hpp"
#include "core/enginestate.hpp"
#include "scene/scene.hpp"
#include "window/window.hpp"

#include <imgui.h>

namespace clz::editor
{
	/**
	 * @brief Renders the main menu bar.
	 *
	 * The menu bar includes:
	 * - File: Save, Exit.
	 * - Edit: Undo, Redo (currently stubs).
	 * - Window controls: minimize, maximize, close (styled as icon buttons).
	 *
	 * @par Decision: Use of custom fonts
	 *      `fontSansBold` and `fontMono` are used to style the menu headers
	 *      and items, giving a distinct look.
	 *
	 * @par Decision: Window controls on the right
	 *      The buttons are manually positioned using `SetCursorPosX` to align
	 *      them to the far right of the menu bar, mimicking typical application
	 *      title bars.
	 *
	 * @par Decision: Transparent button backgrounds
	 *      The buttons are styled with transparent backgrounds and hover/active
	 *      colors to blend with the menu bar.
	 */
	void showTopBar()
	{
		if (!ImGui::BeginMainMenuBar())
			return;

		/// --- file ---
		ImGui::PushFont(fontSansBold);
		const bool fileOpen = ImGui::BeginMenu("File");
		ImGui::PopFont();

		if (fileOpen)
		{
			ImGui::PushFont(fontMono);

			if (ImGui::MenuItem("Save"))
			{
				scene::saveScene();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
			{
				state::g_engineState = state::EngineState::Shutdown;
			}

			ImGui::PopFont();
			ImGui::EndMenu();
		}

		/// --- edit ---
		ImGui::PushFont(fontSansBold);
		const bool editOpen = ImGui::BeginMenu("Edit");
		ImGui::PopFont();

		if (editOpen)
		{
			ImGui::PushFont(fontMono);

			if (ImGui::MenuItem("Undo"))
			{
				// Placeholder for undo functionality.
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Redo"))
			{
				// Placeholder for redo functionality.
			}

			ImGui::PopFont();
			ImGui::EndMenu();
		}

		// Move to the right

		// Push to the right
		float availWidth = ImGui::GetContentRegionAvail().x;
		float buttonSize = ImGui::GetFrameHeight();
		float spacing = 2.0f;
		float totalButtons = 3 * (buttonSize) + 2 * spacing;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availWidth - totalButtons);

		// Styling
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		// Minimize
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.7f));
		if (ImGui::Button("", ImVec2(buttonSize, buttonSize)))
		{
			window::minimizeWindow();
		}
		ImGui::PopStyleColor(2);
		ImGui::SameLine(0, spacing);

		// Maximize
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.96f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 1.0f, 0.7f));
		if (ImGui::Button("", ImVec2(buttonSize, buttonSize)))
		{
			window::maximizeWindow();
		}
		ImGui::PopStyleColor(2);
		ImGui::SameLine(0, spacing);

		// Close
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		if (ImGui::Button("", ImVec2(buttonSize, buttonSize)))
		{
			state::g_engineState = state::EngineState::Shutdown;
		}
		ImGui::PopStyleColor(2);

		ImGui::PopStyleColor(); // transparent button

		ImGui::EndMainMenuBar();
	}

	/**
	 * @brief Sets up the docking space over the main viewport.
	 *
	 * This function is called every frame to ensure the docking layout
	 * is established. It uses `ImGui::DockSpaceOverViewport` with the
	 * central node flag.
	 *
	 * @par Decision: Dockspace is created every frame
	 *      Calling this each frame ensures that the dockspace is always
	 *      available, even if the window is resized or the layout is reset.
	 */
	void setDockSpace()
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	}
} // namespace clz::editor