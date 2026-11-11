/**
 * @file sceneview.hpp
 * @author curl0z
 * @brief Defines the editor's main viewport (scene view) and its drawing function.
 *
 * The main viewport is rendered to an offscreen target that is then displayed
 * in an ImGui window. This file also declares the global offscreen target object.
 */

#pragma once

#include "offscreen/offscreentarget.hpp"

namespace clz::editor
{
	/**
	 * @brief The main viewport's offscreen render target.
	 *
	 * This global object holds the Vulkan image, descriptor set, camera ID,
	 * and extent for the main scene view. It is updated whenever the viewport
	 * is resized.
	 *
	 * @note Access is inline. All editor modules can use this to obtain the
	 *       current viewport texture and associated camera.
	 */
	inline OffscreenImage mainViewportImage;

	/// Initializes scene view resources
	void initializeSceneView(uint32_t width, uint32_t height);

	/// destroys scene view resources
	void destroySceneView();
	/**
	 * @brief Draws the main viewport (scene view) as an ImGui window.
	 * This function creates the tab bar,
	 * and internally calls `drawMainViewPort()` to display the offscreen image.
	 * It handles focus, mouse interaction, and resizing of the offscreen target.
	 */
	void drawSceneView();

} // namespace clz::editor
