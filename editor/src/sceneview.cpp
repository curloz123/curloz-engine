#include "../include/sceneview.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include "../include/offscreen/offscreentarget.hpp"
#include "window/inputmanager.hpp"
#include "window/mouse.hpp"
#include "core/logs.hpp"

namespace clz::editor
{
	void drawMainViewPort();
}
namespace clz::editor
{
	void drawSceneView()
	{
		if (!ImGui::Begin("Curloz Engine"))
		{
			ImGui::End();
			return;
		}
		if (ImGui::BeginTabBar("Scene View"))
		{
			drawMainViewPort();
			ImGui::EndTabBar();
		}
		ImGui::End();
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
			clz::log::debug("wtf");
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
			renderer::updateCameraProjMatrix(mainViewportImage.cameraId);
			mainViewportImage.outDated = true;
		}

		if (ImGui::IsWindowHovered())
		{
			ImGui::SetWindowFocus(ImGui::GetCurrentWindow()->Name);
		}
		if (ImGui::IsWindowFocused())
		{
			static bool rightClickThisFrame = false;
			static bool rightClickLastFrame = false;
			if (window::isMousePressed(clz::input::Mouse::MouseRight))
			{
				rightClickThisFrame = true;
			}
			else
			{
				rightClickThisFrame = false;
			}

			if (rightClickThisFrame)
			{
				const auto Id = mainViewportImage.cameraId;
				renderer::useCamera(Id);
				renderer::updateCamera(Id);
			}

			if (rightClickThisFrame && !rightClickLastFrame)
			{
				window::disableCursor();

			}
			else if (!rightClickThisFrame && rightClickLastFrame)
			{
				window::enableCursor();
				renderer::setCameraFirstTime(mainViewportImage.cameraId);
			}

			rightClickLastFrame = rightClickThisFrame;

		}

		ImGui::Image(
			(ImTextureID)mainViewportImage.descriptorSet,
			avail);

		ImGui::EndTabItem();
	}

}
