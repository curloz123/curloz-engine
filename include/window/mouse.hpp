/**
 * @file mouse.hpp
 * @author curl0z
 * @brief Provides mouse utilities wrapped around GLFW.
 */

#pragma once

#include "window_types.hpp"
#include "core/logs.hpp"

#include <source_location>

namespace clz::window
{
	/// @brief Cursor's position this frame
	/// Is updated by the cursor callback
	inline math::vec2 cursorPosThisFrame = math::vec2(0.0f, 0.0f);
	/// @brief Cursor's position last frame
	/// Is updated by window's main update loop
	inline math::vec2 cursorPosLastFrame = math::vec2(0.0f, 0.0f);
	/// @brief Cursor offset this frame
	/// Calculated by subtracting cursorPosThisFrame with last frame
	inline math::vec2 cursorOffset = math::vec2(0.0f, 0.0f);

	/// @brief Returns cursor offset this frame
	/// @return offset this frame
	inline math::vec2 getCursorOffset()
	{
		return cursorOffset;
	}
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
	 * @brief Hides the cursor and locks it to the window for unlimited relative movement
	 * (e.g., FPS camera look).
	 */
	inline void disableCursor(
	     const std::source_location loc = std::source_location::current())
	{
		glfwSetInputMode(w_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		clz::log::debug("cursor disabled by: ", loc);

		double cursorX = 0;
		double cursorY = 0;
		glfwGetCursorPos(w_window, &cursorX, &cursorY);
		cursorPosThisFrame = math::vec2(
			static_cast<float>(cursorX),
			static_cast<float>(cursorY));
		cursorPosLastFrame = math::vec2(
			static_cast<float>(cursorX),
			static_cast<float>(cursorY));
		cursorOffset = math::vec2(0.0f, 0.0f);

	}

	/**
	 * @brief Restores the cursor to normal behavior and visibility.
	 */
	inline void enableCursor(
	     const std::source_location loc = std::source_location::current())
	{
		glfwSetInputMode(w_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		clz::log::debug("cursor enabled by: ", loc);

		double cursorX = 0;
		double cursorY = 0;
		glfwGetCursorPos(w_window, &cursorX, &cursorY);
		cursorPosThisFrame = math::vec2(
			static_cast<float>(cursorX),
			static_cast<float>(cursorY));
		cursorPosLastFrame = math::vec2(
			static_cast<float>(cursorX),
			static_cast<float>(cursorY));
		cursorOffset = math::vec2(0.0f, 0.0f);
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
	 * @note Calling this function automatically resets the internal scroll offset back
	 * to 0.0f to prevent continuous input processing.
	 * @return float The accumulated scroll offset value.
	 */
	inline float getScrollOffset()
	{
		const float offset = w_scrollOffset;
		w_scrollOffset = 0.0f;
		return offset;
	}
} // namespace clz::window