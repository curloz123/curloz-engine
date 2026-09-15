/**
 * @file system_settings.cpp
 * @author curl0z
 * @brief Shows each subsytem's settings in a seperate window
 */

#include "../../include/system_settings/system_settings.hpp"
#include <imgui.h>

namespace clz::editor
{
	/// @copydoc showSystemSettings
	void showSystemSettings()
	{
		if (!ImGui::Begin("System Settings Window"))
		{
			ImGui::End();
			return;
		}

		if (ImGui::BeginTabBar("System Settings Tab bar"))
		{
			showRenderSystemSettings();
			ImGui::EndTabBar();
		}


		ImGui::End();
	}

}
