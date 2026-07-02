/**
 * @file gizmo.hpp
 * @author curl0z
 * @brief This file contains editor's transform gizmo source code
 * The gizmo can be used to either translate, rotate or scale.
 * Shortcut have also been assigned for same.
 * Press 't' to translate selected entity,
 * Press 'r' to rotate and
 * Press 's' to scale
 *
 * @note must be called inside editor::render() after ImGui::BeginFrame
 * has been called
 */
#pragma once

namespace clz::editor
{
	/**
	 * @brief Draws the 3d transform gizmo
	 */
	void drawGizmo();
}