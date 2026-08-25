#include "../../include/system_settings/system_settings.hpp"
#include <imgui.h>

namespace clz::editor
{
	void showSystemSettings()
	{
		if (!ImGui::Begin("System Settings"))
			return;

		if (ImGui::BeginTabBar("System Settings"))
		{
			showRenderSystemSettings();
			ImGui::EndTabBar();
		}

		ImGui::End();
	}

}
