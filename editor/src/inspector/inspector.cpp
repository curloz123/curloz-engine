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
#include "../../include/inspector/bufferplayer_component.hpp"
#include "entity/componentmanager.hpp"
#include "include/inspector/rigidbodycomponent.hpp"
#include "physics/physicscomponent.hpp"
#include "renderer/rendercomponent.hpp"
#include "audio/audio_components.hpp"
#include <imgui.h>
#include <vulkan/vulkan_core.h>
#include "include/inspector/lightcomponent.hpp"
#include "../../include/inspector/scripts_component.hpp"
#include "script/script_components.hpp"

namespace clz::editor
{
	/// Shows component specific windows
	/// necessary because they need to be visible independently too,
	/// no matter whether Inspector window is opened or not
	static void showComponentSpecificWindows();

	void initInspector()
	{
		bodyEditorImage.initialize(1024, 768);
	}

	void closeInspector()
	{
		bodyEditorImage.destroy();
	}

	/// @copydoc showInspector
	void showInspector(VkCommandBuffer commandBuffer)
	{
		auto showComponents = [&]()
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
				ImGui::Text("Enabled");
			else
				ImGui::Text("Disabled");

			ImGui::PopFont();

			ImGui::Separator();

			// Every entity has transform component
			showTransformComponentHeader();

			ImGui::Separator();

			if (ecs::hasComponent<renderer::ModelComponent>(
				    currentSelectedEntity.value()
			))
			{
				showModelComponentHeader();
				ImGui::Separator();
			}
			if (ecs::hasComponent<renderer::DirectionalLightComponent>(
				currentSelectedEntity.value()
			))
			{
				showDirectionalLightHeader();
				ImGui::Separator();
			}
			if (ecs::hasComponent<renderer::PointLightComponent>(
				currentSelectedEntity.value()
			))
			{
				showPointLightHeader();
				ImGui::Separator();
			}

			if (ecs::hasComponent<physics::RigidBodyComponent>(
				    currentSelectedEntity.value()
			))
			{
				showRigidBodyHeader(commandBuffer);
				ImGui::Separator();
			}

			if (ecs::hasComponent<script::SensorScriptComponent>(currentSelectedEntity.value()))
			{
				showSensorScriptComponent();
				ImGui::Separator();
			}
			if (ecs::hasComponent<script::CollisionScriptComponent>(currentSelectedEntity.value()))
			{
				showCollisionScriptComponent();
				ImGui::Separator();
			}

			if (ecs::hasComponent<audio::AudioBufferPlayerComponent>(currentSelectedEntity.value()))
			{
				showBufferPlayerHeader();
				ImGui::Separator();
			}

		};

		if (ImGui::Begin("Inspector"))
		{
			if (currentSelectedEntity.has_value())
			{
				showComponents();
			}
			else
			{
				ImGui::TextDisabled("No entity selected");
			}


		}
		ImGui::End();

		showComponentSpecificWindows();
	}


	static void showComponentSpecificWindows()
	{
		showRigidBodySpecificWindows();
	}
} // namespace clz::editor
