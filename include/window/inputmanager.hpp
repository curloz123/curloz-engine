/**
 * @file inputmanager.hpp
 * @author curl0z
 * @brief Keyboard input definitions and query functions.
 */

#pragma once

#include "input.hpp"
#include "window_types.hpp"

namespace clz::window
{
	/**
	 * @brief Returns true if the key is currently held down.
	 * @param key The key to query.
	 */
	inline bool isPressed(input::Key key)
	{
		return glfwGetKey(w_window, static_cast<int>(key)) == GLFW_PRESS;
	}

	/**
	 * @brief Returns true if the key was just released.
	 * @param key The key to query.
	 */
	inline bool isReleased(input::Key key)
	{
		return glfwGetKey(w_window, static_cast<int>(key)) == GLFW_RELEASE;
	}

} // namespace clz::window