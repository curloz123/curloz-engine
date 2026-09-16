/**
 * @file window_types.hpp
 * @author curl0z
 *
 * @brief stores all window subsystem's internal global variables
 */

#pragma once

#include "math/vec2.hpp"
#include <GLFW/glfw3.h>

namespace clz::window
{
	/// GLFW window handle
	inline GLFWwindow* w_window;

	/// Cursor screen position
	inline math::vec2 w_cursorPosition;
	/// Cursor Scroll offset
	inline float w_scrollOffset;

	/// @brief Explains in which mode window is.
	/// Fullscreen means exclusive fullscreen mode.
	/// WindowedBorderless is just window but fullscreen.
	/// Windowed is well, just windowed lol.
	enum WindowMode
	{
		FULLSCREEN,
		WINDOWED_BORDERLESS,
		WINDOW
	};
	/// @brief Window Mode handle
	inline WindowMode w_windowMode = WindowMode::WINDOWED_BORDERLESS;

} // namespace clz::window
