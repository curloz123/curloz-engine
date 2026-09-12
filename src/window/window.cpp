/**
 * @file window.cpp
 * @author curl0z
 * @brief Implementation of the window public header
 */

#include "window/config.hpp"
#include "window/window.hpp"
#include "core/logs.hpp"
#include "window/mouse.hpp"
#include "window/native.hpp"
#include "window/vulkanhelper.hpp"
#include "window/window_types.hpp"
#include <GLFW/glfw3.h>
#include <string>

namespace clz::window
{
	/// @copydoc init
	bool init()
	{
		/// --- First of all, parse config data --- ///
		parseConfigData();

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

	/// @copydoc shutdown
	void shutdown()
	{
		shutdownGLFW(&w_window);
	}

	/// @copydoc update
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

	/// @copydoc getFramebufferExtents
	std::tuple<int, int> getFramebufferExtents()
	{
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(w_window, &width, &height);

		return std::make_tuple(width, height);
	}

	/// @copydoc getWindowHandle
	GLFWwindow* getWindowHandle()
	{
		return w_window;
	}

	/// @copydoc minimizeWindow
	void minimizeWindow()
	{
		glfwIconifyWindow(w_window);
	}

	/// @copydoc maximizeWindow
	void toggleWindowedBorderlessMode()
	{
		static bool makeFullScreen = false;
		makeFullScreen = !makeFullScreen;

		if (makeFullScreen)
		{
			glfwSetWindowAttrib(w_window, GLFW_DECORATED, GLFW_FALSE);
			glfwMaximizeWindow(w_window);
		}
		else
		{
			glfwSetWindowAttrib(w_window, GLFW_DECORATED, GLFW_TRUE);
			glfwSetWindowSize(w_window, w_width, w_height);
		}
	}

	/// @brief toggleexclusivefullscreenmode
	void toggleExclusiveFullscreenMode()
	{
		static bool makeFullScreen = false;
		makeFullScreen = !makeFullScreen;

		if (makeFullScreen)
		{
			glfwSetWindowAttrib(w_window, GLFW_DECORATED, GLFW_FALSE);
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(
				w_window,
				monitor,
				0,
				0,
				mode->width,
				mode->height,
				mode->refreshRate
			);
		}
		else
		{
			glfwSetWindowMonitor(
				w_window,
				nullptr,
				0,
				0,
				w_width,
				w_height,
				GLFW_DONT_CARE
			);
			glfwSetWindowAttrib(w_window, GLFW_DECORATED, GLFW_TRUE);
		}
		
	}
} // namespace clz::window
