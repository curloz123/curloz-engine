/**
 * @file sceneview.cpp
 * @brief Implementation of the editor's main scene viewport.
 *
 * This file contains the logic to display the offscreen render target
 * as an ImGui image, handle viewport resizing, camera focus, and
 * input forwarding (right‑click to control the camera).
 */

#include "../include/sceneview.hpp"
#include "../include/gizmo/entitytransformgizmo.hpp"
#include "../include/offscreen/offscreentarget.hpp"
#include "core/logs.hpp"
#include "include/inspector/rigidbodycomponent.hpp"
#include "window/inputmanager.hpp"
#include "window/mouse.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace clz::editor
{
	// Forward declaration of the internal viewport drawing function.
	void drawMainViewPort();
} // namespace clz::editor

namespace clz::editor
{
	/**
	 * @brief Draws the "Curloz Engine" window and its tab bar.
	 *
	 * This is the entry point for the scene view. It creates the top‑level
	 * window and a tab bar, then delegates to `drawMainViewPort()`.
	 *
	 * @par Decision: Tab bar for future multi‑view support
	 *      Even though currently only one tab ("Game view") exists, the tab
	 *      bar structure allows adding other views later.
	 *      All other offscreen images shall be drawn here only.
	 */
	void drawSceneView()
	{
		if (!ImGui::Begin("Curloz Engine"))
		{
			ImGui::End();
			return;
		}
		/// --- Main Scene
		if (mainViewportImage.showTarget)
			drawMainViewPort();

		if (physicsBodyShapeImage.showTarget)
			presentBodyEditorWindow();


		/// --- Rigid body editor
		ImGui::End();
	}
} // namespace clz::editor

namespace clz::editor
{
	/**
	 * @brief Draws the actual viewport tab and handles interaction.
	 *
	 * This function:
	 * - Retrieves the available area from ImGui.
	 * - Resizes the offscreen target if the area changed and the mouse is released.
	 * - Manages window focus and right‑click camera control.
	 * - Displays the offscreen image as an ImGui texture.
	 * - Calls the entity transform gizmo on top of the viewport.
	 *
	 * @par Decision: Resize only on mouse release
	 *      To avoid constant resizing during dragging, the offscreen target
	 *      is resized only when the left mouse button is released. This
	 *      reduces performance overhead and flickering.
	 *
	 * @par Decision: Right‑click focus to control camera
	 *      When the viewport is focused and the right mouse button is pressed,
	 *      the cursor is disabled and the camera is updated. This allows
	 *      the user to orbit/zoom the scene view. When right‑click is released,
	 *      the cursor is re‑enabled and the camera’s first‑time flag is reset.
	 *
	 * @par Decision: Gizmo is drawn after the image
	 *      The gizmo is overlaid on top of the viewport image, using the
	 *      same rectangle. This ensures correct hit‑testing and visual layering.
	 */
	void drawMainViewPort()
	{
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		if (avail.x < 1.0f || avail.y < 1.0f)
		{
			clz::log::debug("wtf");
			return;
		}
		const auto width = static_cast<uint32_t>(avail.x);
		const auto height = static_cast<uint32_t>(avail.y);
		if ((width != mainViewportImage.extent.width || height != mainViewportImage.extent.height) &&
		    clz::window::isMouseReleased(clz::input::Mouse::MouseLeft))
		{
			mainViewportImage.extent.width = width;
			mainViewportImage.extent.height = height;
			renderer::updateCameraProjMatrix(mainViewportImage.cameraId);
			mainViewportImage.outDated = true;
		}

		/*
		if (ImGui::IsWindowHovered())
		{
			ImGui::SetWindowFocus(ImGui::GetCurrentWindow()->Name);
		}
		*/
		if (ImGui::IsWindowFocused())
		{
			static bool rightClickThisFrame = false;
			static bool rightClickLastFrame = false;

			if (window::isMousePressed(clz::input::Mouse::MouseRight))
				rightClickThisFrame = true;
			else
				rightClickThisFrame = false;

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

		const ImVec2 cursorPosBefore = ImGui::GetCursorScreenPos();

		ImGui::Image((ImTextureID)mainViewportImage.descriptorSet, avail);

		const Rect2D rect{.x = static_cast<uint32_t>(cursorPosBefore.x),
				  .y = static_cast<uint32_t>(cursorPosBefore.y),
				  .width = mainViewportImage.extent.width,
				  .height = mainViewportImage.extent.height};
		drawEntityTransformGizmo(rect);

	}

} // namespace clz::editor