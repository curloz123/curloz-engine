#include "../include/sceneview.hpp"
#include <imgui.h>
#include "../include/offscreen/offscreentarget.hpp"
#include "window/inputmanager.hpp"

namespace clz::editor
{
	void drawMainViewPort();
}
namespace clz::editor
{
	void drawSceneView()
	{
		if (ImGui::BeginTabBar("Scene View"))
		{
			drawMainViewPort();
			ImGui::EndTabBar();
		}
	}
}

namespace clz::editor
{
	void drawMainViewPort()
	{
		if (!ImGui::BeginTabItem("Game view"))
		{
			ImGui::EndTabItem();
			return;
		}
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		if (avail.x < 1.0f || avail.y < 1.0f)
		{
			ImGui::EndTabItem();
			return;
		}
		const auto width = static_cast<uint32_t>(avail.x);
		const auto height = static_cast<uint32_t>(avail.y);
		if ((width != mainViewportImage.extent.width ||
			height != mainViewportImage.extent.height) &&
			clz::window::isMouseReleased(clz::input::Mouse::MouseLeft))
		{
			mainViewportImage.extent.width = width;
			mainViewportImage.extent.height = height;
			mainViewportImage.outDated = true;
		}

		ImGui::Image(
			(ImTextureID)mainViewportImage.descriptorSet,
			avail);

		ImGui::EndTabItem();
	}

}
