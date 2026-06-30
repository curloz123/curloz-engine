/**
 * @file window.hpp
 * @author curl0z
 * @brief Public interface for the window subsystem.
 *
 * Wraps GLFW window creation and event polling.
 * Depends on the config subsystem being initialized first.
 *
 * @note config::init() must be called before window::init().
 */

#pragma once
#include "math/vec2.hpp"

namespace clz::window
{
	/**
	 * @brief Initializes GLFW and creates the window using config values.
	 * @note Logs an error and returns early on failure.
	 */
	void init();

	/**
	 * @brief Destroys the window and terminates GLFW.
	 */
	void shutdown();

	/**
	 * @brief Polls GLFW events and checks window close state.
	 *
	 * Sets g_engineState to EngineState::Shutdown if the window is closed
	 */
	void update();

	/**
	 * @brief Returns actual framebuffer extents
	 * Basically coords in pixel space not screen space
	 * @param width
	 * @param height
	 */
	void getFramebufferExtents(int* width, int* height);

} // namespace clz::window