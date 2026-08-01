/**
 * @file topbar.hpp
 * @brief Declares the editor's main menu bar and docking system.
 *
 * Provides functions to render the top menu bar (File, Edit, window controls)
 * and to set up the ImGui docking space for the editor layout.
 */

#pragma once

namespace clz::editor
{
/**
 * @brief Renders the main menu bar at the top of the editor window.
 *
 * This function creates the "File" and "Edit" menus, and adds
 * minimise, maximise, and close buttons on the right side.
 *
 * @par Decision: Window controls
 *      The close and minimise buttons directly manipulate the engine state
 *      (`state::g_engineState`) or call the window system, providing a
 *      consistent way to exit or resize the editor.
 */
void showTopBar();

/**
 * @brief Sets up the ImGui docking space over the main viewport.
 *
 * This function creates a central docking node that allows other ImGui
 * windows (Scene View, Inspector, etc.) to be docked.
 *
 * @par Decision: Central docking node
 *      Using `ImGuiDockNodeFlags_PassthruCentralNode` ensures that the
 *      main viewport area is used as a dockable space, making the editor
 *      layout flexible.
 */
void setDockSpace();
} // namespace clz::editor