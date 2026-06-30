/**
 * @file window.cpp
 * @author curl0z
 * @brief Implementation of the window public header
 */
#include "window/window.hpp"
#include "core/logs.hpp"
#include "window/mouse.hpp"
#include "window/native.hpp"
#include "window/vulkanhelper.hpp"
#include "window/window_types.hpp"

namespace clz::window
{
	void init()
	{
		// Initialize window
		if (!initializeGLFW(&w_window))
		{
			log::error("COuld not create window");
			return;
		}

		// Initialize all callback functions
		glfwSetFramebufferSizeCallback(w_window, hintRendererAboutResize);
		glfwSetCursorPosCallback(w_window, cursorCallback);
		glfwSetScrollCallback(w_window, scrollCallback);

		clz::log::debug("Initialized window system");
	}

	void shutdown()
	{
		shutdownGLFW(&w_window);
	}

	void update()
	{
		pollEventsGLFW(&w_window);
	}

	// Other subsystems helper functions
	void getFramebufferExtents(int* width, int* height)
	{
		glfwGetFramebufferSize(w_window, width, height);
	}

} // namespace clz::window
