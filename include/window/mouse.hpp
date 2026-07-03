/**
 * @file mouse.hpp
 * @author curl0z
 * @brief Provides mouse utilities wrapped around GLFW.
 */

#pragma once

#include "window_types.hpp"

namespace clz::window
{
	/**
	 * @brief GLFW callback function for tracking cursor movement.
	 * @param window The GLFW window instance that received the event.
	 * @param xPos The new absolute X-coordinate of the cursor.
	 * @param yPos The new absolute Y-coordinate of the cursor.
	 */
	void cursorCallback(GLFWwindow* window, double xPos, double yPos);

	/**
	 * @brief GLFW callback function for tracking scroll wheel input.
	 * @param window The GLFW window instance that received the event.
	 * @param xOffset The scroll offset along the X axis.
	 * @param yOffset The scroll offset along the Y axis.
	 */
	void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

	/**
	 * @brief Hides the cursor and locks it to the window for unlimited relative movement (e.g., FPS camera look).
	 */
	inline void disableCursor()
	{
		glfwSetInputMode(w_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	/**
	 * @brief Restores the cursor to normal behavior and visibility.
	 */
	inline void enableCursor()
	{
		glfwSetInputMode(w_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	/**
	 * @brief Retrieves the current screen-space coordinates of the mouse cursor.
	 * @return math::vec2 A 2D vector containing the current mouse position.
	 */
	inline math::vec2 getCursorPosition()
	{
		return w_cursorPosition;
	}

	/**
	 * @brief Retrieves the scroll offset accumulated since the last check.
	 * @note Calling this function automatically resets the internal scroll offset back to 0.0f
	 * to prevent continuous input processing.
	 * @return float The accumulated scroll offset value.
	 */
	inline float getScrollOffset()
	{
		const float offset = w_scrollOffset;
		w_scrollOffset = 0.0f;
		return offset;
	}
} // namespace clz::window