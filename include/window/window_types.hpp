/**
 * @file window_types.hpp
 * @author curl0z
 *
 * @brief stores all window subsystem's internal global variables
 */

#pragma once

#include <GLFW/glfw3.h>
#include "math/vec2.hpp"

namespace clz::window
{
	/// GLFW window handle
	inline GLFWwindow* w_window;

	/// Cursor screen position
	inline math::vec2 w_cursorPosition;
	/// Cursor Scroll offset
	inline float w_scrollOffset;
}