/**
 * @file native.hpp
 * @author curl0z
 * @brief Low level GLFW initialization and teardown functions.
 *
 * @note These are internal to the window subsystem — do not call directly.
 * Use the public window.hpp interface instead.
 */

#pragma once

#include "GLFW/glfw3.h"

namespace clz::window
{
	/**
	 * @brief Initializes GLFW and creates the window.
	 *
	 * @note Reads window dimensions from the config subsystem.
	 *
	 * @param pWindow pointer to the GLFW window
	 * @return true on success, false on failure and logs an error
	 */
	bool initializeGLFW(GLFWwindow** pWindow);

	/**
	 * @brief Destroys the GLFW window and terminates GLFW.
	 * @param pWindow pointer to the window handle to destroy.
	 */
	void shutdownGLFW(GLFWwindow** pWindow);

	/**
	 * @brief Polls GLFW events and checks for window close or escape key.
	 * @param pWindow pointer to the active window.
	 */
	void pollEventsGLFW(GLFWwindow** pWindow);

} // namespace clz::window
