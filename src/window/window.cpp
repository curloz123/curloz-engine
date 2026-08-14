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

		/// --- Initialize cursor ---
		double cursorX = 0;
		double cursorY = 0;
		glfwGetCursorPos(w_window, &cursorX, &cursorY);
		cursorPosThisFrame = math::vec2(
			static_cast<float>(cursorX),
			static_cast<float>(cursorY));
		cursorPosLastFrame = math::vec2(
			static_cast<float>(cursorX),
			static_cast<float>(cursorY));

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

		/// update cursor offset
		cursorOffset = cursorPosThisFrame - cursorPosLastFrame;
		if (std::abs(cursorOffset.x) > 50.0f || std::abs(cursorOffset.y) > 50.0f)
		{
			clz::log::debug("HUGE cursor offset: " + std::to_string(cursorOffset.x) + ", "
				+ std::to_string(cursorOffset.y) + " | this: " +std::to_string(cursorPosThisFrame.x) +  "," +
				std::to_string(cursorPosThisFrame.y) +
				" | last: " + std::to_string(cursorPosLastFrame.x) + "," + std::to_string(cursorPosLastFrame.y));
		}
		cursorPosLastFrame = cursorPosThisFrame;
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
