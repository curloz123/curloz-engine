#include "../include/topbar.hpp"
#include "../include/editor_types.hpp"
#include "core/enginestate.hpp"
#include "scene/scene.hpp"
#include "window/window.hpp"

#include <imgui.h>

namespace clz::editor
{
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
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Redo"))
			{
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
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));

        // Minimize
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f,0.2f,0.2f,0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f,0.3f,0.3f,0.7f));
        if (ImGui::Button("", ImVec2(buttonSize, buttonSize)))
        {
	        window::minimizeWindow();
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine(0, spacing);

        // Maximize
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f,0.59f,0.96f,0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f,0.0f,1.0f,0.7f));
        if (ImGui::Button("", ImVec2(buttonSize, buttonSize)))
        {
		window::maximizeWindow();
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine(0, spacing);

        // Close
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f,0.1f,0.1f,0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f,0.2f,0.2f,1.0f));
        if (ImGui::Button("", ImVec2(buttonSize, buttonSize)))
        {
        	state::g_engineState = state::EngineState::Shutdown;
        }
        ImGui::PopStyleColor(2);

        ImGui::PopStyleColor(); // transparent button

		ImGui::EndMainMenuBar();
	}

	void setDockSpace()
	{
		ImGui::DockSpaceOverViewport(
			ImGui::GetMainViewport()->ID,
			ImGui::GetMainViewport(),
			ImGuiDockNodeFlags_PassthruCentralNode);
	}
}