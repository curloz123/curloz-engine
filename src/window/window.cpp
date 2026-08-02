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
#include <GLFW/glfw3.h>

namespace clz::window
{
/// @copydoc
bool init()
{
	// Initialize window
	if (!initializeGLFW(&w_window))
	{
		log::error("Could not create window");
		return false;
	}

	// Initialize resizing
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Initialize all callback functions
	glfwSetFramebufferSizeCallback(w_window, hintRendererAboutResize);
	glfwSetCursorPosCallback(w_window, cursorCallback);
	glfwSetScrollCallback(w_window, scrollCallback);

	// Cursor's initial state
#ifdef CLZ_ENABLE_EDITOR
	enableCursor();
#else
	disableCursor();
#endif

	clz::log::info("Initialized window system");
	return true;
}

/// @copydoc
void shutdown()
{
	shutdownGLFW(&w_window);
}

/// @copydoc
void update()
{
	pollEventsGLFW(&w_window);
}

/// @copydoc
void getFramebufferExtents(int* width, int* height)
{
	glfwGetFramebufferSize(w_window, width, height);
}

/// @copydoc
GLFWwindow* getWindowHandle()
{
	return w_window;
}

/// @copydoc
void minimizeWindow()
{
	glfwIconifyWindow(w_window);
}

/// @copydoc
void maximizeWindow()
{
	glfwMaximizeWindow(w_window);
}

} // namespace clz::window