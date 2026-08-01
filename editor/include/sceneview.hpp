/**
 * @file sceneview.hpp
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
 * @note Access is inline – all editor modules can use this to obtain the
 *       current viewport texture and associated camera.
 */
inline OffscreenTarget mainViewportImage{};

/**
 * @brief Draws the main viewport (scene view) as an ImGui window.
 *
 * This function creates the "Curloz Engine" window with a tab bar,
 * and internally calls `drawMainViewPort()` to display the offscreen image.
 * It handles focus, mouse interaction, and resizing of the offscreen target.
 *
 * @par Decision: Separate tab for game view
 *      The viewport is placed inside a tab bar to allow future addition of
 *      other views (e.g., editor camera view, UV view, etc.).
 */
void drawSceneView();
} // namespace clz::editor