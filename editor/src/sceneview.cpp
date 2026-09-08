/**
 * @file sceneview.cpp
 * @brief Implementation of the editor's main scene viewport.
 *
 * This file contains the logic to display the offscreen render target
 * as an ImGui image, handle viewport resizing, camera focus, and
 * input forwarding (right‑click to control the camera).
 */

#include "../include/sceneview.hpp"
#include "../include/offscreen/offscreentarget.hpp"
#include "../include/timemachine.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "include/gizmo/gizmo.hpp"
#include "renderer/vk_types.hpp"
#include "include/scenetable.hpp"
#include "renderer/camera/camera.hpp"
#include "window/inputmanager.hpp"
#include "window/mouse.hpp"
#include <X11/X.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace clz::editor
{
	// Forward declaration of the internal viewport drawing function.
	static void drawMainViewPort();
	static void showTransformGizmo(const Rect2D& rect);
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

		/// --- Rigid body editor
		ImGui::End();
	}
} // namespace clz::editor

namespace clz::editor
{
	/// @brief Draws the main viewport
	/// @note Updates camera only on focus, but mouse's right-click is checked
	/// every frame in order to avoid camera-snap
	void drawMainViewPort()
	{
		const ImVec2 availableRegion = ImGui::GetContentRegionAvail();
		if (availableRegion.x < 1.0f || availableRegion.y < 1.0f)
		{
			return;
		}
		// const auto width = static_cast<uint32_t>(availableRegion.x);
		// const auto height = static_cast<uint32_t>(availableRegion.y);
		// if ((width != mainViewportImage.extent.width ||
		//      height != mainViewportImage.extent.height) &&
		//     clz::window::isMouseReleased(clz::input::Mouse::MouseLeft))
		// {
		// 	mainViewportImage.extent.width = width;
		// 	mainViewportImage.extent.height = height;
		// 	renderer::updateCameraProjMatrix(mainViewportImage.cameraId);
		// 	mainViewportImage.outDated = true;
		// }


		static bool rightClickThisFrame = false;
		static bool rightClickLastFrame = false;

		if (ImGui::IsWindowHovered() &&
			window::isMousePressed(clz::input::Mouse::MouseRight))
		{
			rightClickThisFrame = true;
			ImGuiWindow* window = ImGui::GetCurrentContext()->HoveredWindow;
			ImGui::FocusWindow(window);
		}
		else
		{
			rightClickThisFrame = false;
		}

		if (ImGui::IsWindowFocused())
		{
			if (rightClickThisFrame && !rightClickLastFrame)
			{
				window::disableCursor();
			}
			else if (!rightClickThisFrame && rightClickLastFrame)
			{
				window::enableCursor();
			}

			if (rightClickThisFrame)
			{
				renderer::updateCamera(mainViewportImage.cameraId);
			}

			rightClickLastFrame = rightClickThisFrame;
		}


		const ImVec2 cursorPosBefore = ImGui::GetCursorScreenPos();
		const ImVec2 canvasSize = availableRegion;
		const float imageAspect = (float)mainViewportImage.extent.width / 
						(float)mainViewportImage.extent.height;
		const float canvasAspect = canvasSize.x / canvasSize.y;

		ImVec2 finalCanvasSize;
		if (imageAspect > canvasAspect)
		{
			finalCanvasSize.x = canvasSize.x;
			finalCanvasSize.y = canvasSize.x / imageAspect;
		}
		else
		{
			finalCanvasSize.y = canvasSize.y;
			finalCanvasSize.x = canvasSize.y * imageAspect;
		}
		ImVec2 offset = {
			(canvasSize.x - finalCanvasSize.x) * 0.5f,
			(canvasSize.y - finalCanvasSize.y) * 0.5f
		};
		ImVec2 currectPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(
				currectPos.x + offset.x,
				currectPos.y + offset.y)
		);

		ImGui::Image((ImTextureID)mainViewportImage.descriptorSet, finalCanvasSize);

		const Rect2D rect{
			.x = static_cast<uint32_t>(cursorPosBefore.x),
			.y = static_cast<uint32_t>(cursorPosBefore.y),
			.width = mainViewportImage.extent.width,
			.height = mainViewportImage.extent.height
		};
		showTransformGizmo(rect);
	}

	void showTransformGizmo(const Rect2D& rect)
	{
		if (!currentSelectedEntity.has_value())
			return;

		/// --- static variables ---
		static bool gizmoUsedLastFrame = false;
		static bool gizmoUsedThisFrame = false;
		static auto previousGizmoEditorTransform = 
			ecs::getComponent<ecs::EditorTransformComponent>(currentSelectedEntity.value());
		static auto previousGizmoTransform = 
			ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());

		auto proj = renderer::getCameraProjMatrix(
				mainViewportImage.cameraId, 
				mainViewportImage.extent.width,
				mainViewportImage.extent.height);
		auto view = renderer::getCameraViewMatrix(mainViewportImage.cameraId);
		const auto currentTransform = ecs::getComponent<ecs::TransformComponent>(currentSelectedEntity.value());
		const auto [newTransform, newEditorTransform, change] = gizmoTransform(rect, proj, view, currentTransform);


		///< @brief determines if gizmo was changed this frame
		///< @note at the end of this function, gizmoUsedLastFrame is set
		gizmoUsedThisFrame = change;	

		if (change)
		{
			gizmoUsedThisFrame = true;
			if (!gizmoUsedLastFrame)
			{
				previousGizmoTransform = currentTransform;
				previousGizmoEditorTransform = ecs::EditorTransformComponent(currentTransform);
			}
			ecs::setComponent<ecs::TransformComponent>(
					currentSelectedEntity.value(), newTransform);
			ecs::setComponent<ecs::EditorTransformComponent>(
					currentSelectedEntity.value(), newEditorTransform);
		}
		else
		{
			gizmoUsedThisFrame = false;
			if (gizmoUsedLastFrame)
			{
				const auto entityId = currentSelectedEntity.value();
				auto oldTransform = previousGizmoTransform;
				auto oldEditorTransform = previousGizmoEditorTransform;
				timemachine::createSnapshot(
					[entityId, oldTransform, oldEditorTransform] {
						ecs::setComponent<ecs::TransformComponent>(
							entityId,
							oldTransform
						);
						ecs::setComponent<ecs::EditorTransformComponent>(
							entityId,
							oldEditorTransform
						);
					},
					[entityId, newTransform, newEditorTransform] {
						ecs::setComponent<ecs::TransformComponent>(
							entityId,
							newTransform
						);
						ecs::setComponent<ecs::EditorTransformComponent>(
							entityId,
							newEditorTransform
						);
					}
				);
			}
		}

		/// @brief Swap gizmo frame checkers
		gizmoUsedLastFrame = gizmoUsedThisFrame;
	}
} // namespace clz::editor
