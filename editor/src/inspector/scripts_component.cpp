/**
 * @file scripts_component.cpp
 * @author curl0z
 * @brief Displays all script-related components
 */

#include <filesystem>
#include <imgui.h>
#include "../../include/inspector/scripts_component.hpp"
#include "include/scenetable.hpp"
#include "script/script_components.hpp"
#include "entity/componentmanager.hpp"
#include <ImGuiFileDialog.h>
#include "../../include/editor_types.hpp"
#include "../../include/timemachine.hpp"

namespace clz::editor
{
	/// @copydoc showSensorScriptComponent
	void showSensorScriptComponent()
	{
		if (!ImGui::CollapsingHeader("󰯁 Sensor Script"))
			return;

		auto& sensorScriptComponent = ecs::getComponent<
			script::SensorScriptComponent>(currentSelectedEntity.value());

		std::vector<std::string> currentSensorScripts;
		sensorScriptComponent.getScriptPaths(currentSensorScripts);
		
		ImGui::PushFont(fontMono);
		for (auto scriptPath : currentSensorScripts)
		{
			ImGui::PushID(scriptPath.c_str());
			ImGui::Text("%s", scriptPath.c_str());

			const float buttonWidth = ImGui::GetFrameHeight();
			ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - buttonWidth);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f,0.1f,0.1f,1.0f));

			if (ImGui::Button("", ImVec2(buttonWidth, buttonWidth)))
			{
			    	sensorScriptComponent.removeScript(scriptPath);

				/// --- create snapshot --- ///
				auto sensorEntity = currentSelectedEntity.value();
				timemachine::createSnapshot(
					[scriptPath, sensorEntity]()
					{
						if (!ecs::hasComponent<
							script::SensorScriptComponent>(
								sensorEntity)) [[unlikely]]
						{
							return;
						}
						auto& sensorScriptComponent = ecs::getComponent<
							script::SensorScriptComponent>(sensorEntity);
						std::filesystem::path uScriptPath(scriptPath);
						sensorScriptComponent.loadScript(uScriptPath);
					},
					[scriptPath, sensorEntity]()
					{
						if (!ecs::hasComponent<
							script::SensorScriptComponent>(
								sensorEntity)) [[unlikely]]
						{
							return;
						}
						auto& sensorScriptComponent = ecs::getComponent<
							script::SensorScriptComponent>(sensorEntity);
						std::filesystem::path rScriptPath(scriptPath);
						sensorScriptComponent.removeScript(rScriptPath);
					}
				);
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Remove Script");

			ImGui::PopStyleColor(2);

			ImGui::PopID();
			ImGui::Separator();
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f,0.5f,0.5f,0.5f));
		ImGui::PushFont(fontSansBold, 30);
		constexpr auto pickScriptDlg = "PickScriptDialog";
		if (ImGui::Button("+", ImVec2(-1, 40)))
		{
			IGFD::FileDialogConfig config;
			config.path = "assets/scripts";
			ImGuiFileDialog::Instance()->OpenDialog(
					pickScriptDlg, 
					"Select Sensor Script", 
					".lua", 
					config);
		}
		ImGui::PopFont();
		ImGui::PushFont(fontMono);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Add Script");
		ImGui::PopFont();
		
		ImGui::PopStyleColor(2);


		if (ImGuiFileDialog::Instance()->Display(pickScriptDlg))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				const std::string absolutePath = ImGuiFileDialog::Instance()->GetFilePathName();	
				const auto relativePath = std::filesystem::relative(
							absolutePath,
							std::filesystem::current_path());
				clz::log::debug("Adding script: " + relativePath.string());
				sensorScriptComponent.loadScript(relativePath);

				/// --- create snapshot --- ///
				auto sensorEntity = currentSelectedEntity.value();
				timemachine::createSnapshot(
					[relativePath, sensorEntity]()
					{
						if (!ecs::hasComponent<
							script::SensorScriptComponent>(
								sensorEntity)) [[unlikely]]
						{
							return;
						}
						auto& sensorScriptComponent = ecs::getComponent<
							script::SensorScriptComponent>(sensorEntity);
						std::filesystem::path uScriptPath(relativePath);
						sensorScriptComponent.removeScript(uScriptPath);
					},
					[relativePath, sensorEntity]()
					{
						if (!ecs::hasComponent<
							script::SensorScriptComponent>(
								sensorEntity)) [[unlikely]]
						{
							return;
						}
						auto& sensorScriptComponent = ecs::getComponent<
							script::SensorScriptComponent>(sensorEntity);
						std::filesystem::path rScriptPath(relativePath);
						sensorScriptComponent.loadScript(rScriptPath);
					}
				);

			}
			ImGuiFileDialog::Instance()->Close();
		}

		ImGui::PopFont();
	}
	/// @copydoc showCollisionScriptComponent
	void showCollisionScriptComponent()
	{
		if (!ImGui::CollapsingHeader(" Collision Script"))
			return;

		auto& collisionScriptComponent = ecs::getComponent<
			script::CollisionScriptComponent>(currentSelectedEntity.value());

		std::vector<std::string> currentCollisionScripts;
		collisionScriptComponent.getScriptPaths(currentCollisionScripts);
		
		ImGui::PushFont(fontMono);
		for (auto scriptPath : currentCollisionScripts)
		{
			ImGui::PushID(scriptPath.c_str());
			ImGui::Text("%s", scriptPath.c_str());

			const float buttonWidth = ImGui::GetFrameHeight();
			ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - buttonWidth);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f,0.1f,0.1f,1.0f));

			if (ImGui::Button("", ImVec2(buttonWidth, buttonWidth)))
			{
			    	collisionScriptComponent.removeScript(scriptPath);

				/// --- create snapshot --- ///
				auto scriptEntity = currentSelectedEntity.value();
				timemachine::createSnapshot(
					[scriptPath, scriptEntity]()
					{
						if (!ecs::hasComponent<
							script::CollisionScriptComponent>(
								scriptEntity)) [[unlikely]]
						{
							return;
						}
						auto& collisionScriptComponent = ecs::getComponent<
							script::CollisionScriptComponent>(scriptEntity);
						std::filesystem::path uScriptPath(scriptPath);
						collisionScriptComponent.loadScript(uScriptPath);
					},
					[scriptPath, scriptEntity]()
					{
						if (!ecs::hasComponent<
							script::CollisionScriptComponent>(
								scriptEntity)) [[unlikely]]
						{
							return;
						}
						auto& collisionScriptComponent = ecs::getComponent<
							script::CollisionScriptComponent>(scriptEntity);
						std::filesystem::path rScriptPath(scriptPath);
						collisionScriptComponent.removeScript(rScriptPath);
					}
				);
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Remove Script");

			ImGui::PopStyleColor(2);

			ImGui::PopID();
			ImGui::Separator();
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f,0.5f,0.5f,0.5f));
		ImGui::PushFont(fontSansBold, 30);
		constexpr auto pickScriptDlg = "PickScriptDialog";
		if (ImGui::Button("+", ImVec2(-1, 40)))
		{
			IGFD::FileDialogConfig config;
			config.path = "assets/scripts";
			ImGuiFileDialog::Instance()->OpenDialog(
					pickScriptDlg, 
					"Select Collision Script", 
					".lua", 
					config);
		}
		ImGui::PopFont();
		ImGui::PushFont(fontMono);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Add Script");
		ImGui::PopFont();
		
		ImGui::PopStyleColor(2);


		if (ImGuiFileDialog::Instance()->Display(pickScriptDlg))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				const std::string absolutePath = ImGuiFileDialog::Instance()->GetFilePathName();	
				const auto relativePath = std::filesystem::relative(
							absolutePath,
							std::filesystem::current_path());
				clz::log::debug("Adding script: " + relativePath.string());
				collisionScriptComponent.loadScript(relativePath);

				/// --- create snapshot --- ///
				auto scriptEntity = currentSelectedEntity.value();
				timemachine::createSnapshot(
					[relativePath, scriptEntity]()
					{
						if (!ecs::hasComponent<
							script::CollisionScriptComponent>(
								scriptEntity)) [[unlikely]]
						{
							return;
						}
						auto& collisionScriptComponent = ecs::getComponent<
							script::CollisionScriptComponent>(scriptEntity);
						std::filesystem::path uScriptPath(relativePath);
						collisionScriptComponent.removeScript(uScriptPath);
					},
					[relativePath, scriptEntity]()
					{
						if (!ecs::hasComponent<
							script::CollisionScriptComponent>(
								scriptEntity)) [[unlikely]]
						{
							return;
						}
						auto& collisionScriptComponent = ecs::getComponent<
							script::CollisionScriptComponent>(scriptEntity);
						std::filesystem::path rScriptPath(relativePath);
						collisionScriptComponent.loadScript(rScriptPath);
					}
				);

			}
			ImGuiFileDialog::Instance()->Close();
		}

		ImGui::PopFont();
	}

}
