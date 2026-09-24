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
#include "include/scenetable.hpp"
#include "renderer/camera/camera.hpp"
#include "window/inputmanager.hpp"
#include "window/mouse.hpp"
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

		/// Identifies whether
		/// (player viewing) || (Right mouse button is pressed) or not
		static bool isLooking = false;

		/// --- Is Right mouse button pressed this frame???? ---
		const bool RMousePressed = window::isMousePressed(clz::input::Mouse::MouseRight);

		/// If user suddenly brings mouse over scene view and right-clicks,
		/// Make scene window the current focused window
		if (!isLooking && ImGui::IsWindowHovered() && RMousePressed)
		{
			ImGuiWindow* window = ImGui::GetCurrentContext()->HoveredWindow;
			ImGui::FocusWindow(window);
		}

		/// --- update if started viewing this frame, disable cursor --- ///
		if (!isLooking && ImGui::IsWindowFocused() && RMousePressed)
		{
			isLooking = true;
			window::disableCursor();
		}
		/// --- Else enable cursor --- ///
		if (isLooking && !RMousePressed)
		{
			isLooking = false;
			window::enableCursor();
		}
		
		/// --- If looking right now, only then update camera --- ///
		if (isLooking)
		{
			renderer::updateCamera(mainViewportImage.cameraId);
		}


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
		ImVec2 currentPosLocal  = ImGui::GetCursorPos();      
		ImVec2 currentPosScreen = ImGui::GetCursorScreenPos();

		ImVec2 newCursorPosLocal  = ImVec2(currentPosLocal.x + offset.x,  currentPosLocal.y + offset.y);
		ImVec2 newCursorPosScreen = ImVec2(currentPosScreen.x + offset.x, currentPosScreen.y + offset.y);

		ImGui::SetCursorPos(newCursorPosLocal);
		ImGui::Image((ImTextureID)mainViewportImage.descriptorSet, finalCanvasSize);

		const Rect2D rect{
			.x = static_cast<uint32_t>(newCursorPosScreen.x),
			.y = static_cast<uint32_t>(newCursorPosScreen.y),
			.width = static_cast<uint32_t>(finalCanvasSize.x),
			.height = static_cast<uint32_t>(finalCanvasSize.y),
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

	/// @copydoc getEditorMainViewCameraId
	renderer::CameraId getEditorMainViewCameraId()
	{
		return mainViewportImage.cameraId;
	}
} // namespace clz::editor
