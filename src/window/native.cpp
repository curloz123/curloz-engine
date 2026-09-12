/**
 * @file native.cpp
 * @author curl0z
 * @brief implements all the internal GLFW functions
 */

#include "window/native.hpp"
#include "window/config.hpp"
#include "config/config.hpp"
#include "core/enginestate.hpp"
#include "core/logs.hpp"

namespace clz::window
{
	/// @copydoc
	bool initializeGLFW(GLFWwindow** pWindow)
	{
		if (w_width < 0 || w_height < 0)
		{
			log::error("Window system passed invalid window dimensions");
			return false;
		}

		if (!glfwInit())
		{
			log::error("Could not initialize GLFW");
			return false;
		}

		/// --- Disable opengl context --- ///
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		/// --- Enable resizability --- ///
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		/// --- Have full screen mode?? --- ///
		GLFWmonitor* monitor = nullptr;
		if (w_exclusiveFullscreen)
		{
			monitor = glfwGetPrimaryMonitor();
		}

		/// --- create window finally --- ///
		*pWindow = glfwCreateWindow(
			w_width,
			w_height,
			clz::config::getValue<std::string>(
				"engine", 
				"name", 
				"Curloz Engine").c_str(),
			monitor,
			nullptr
		);
		if (!(*pWindow))
		{
			log::error("Could not create GLFW window");
			return false;
		}

		/// --- Enable Raw mouse input if enabled--- ///
		if (glfwRawMouseMotionSupported() && w_enableRawInput)
		{
			glfwSetInputMode((*pWindow), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}


		return true;
	}

	/// @copydoc
	void shutdownGLFW(GLFWwindow** pWindow)
	{
		glfwDestroyWindow(*pWindow);
		pWindow = nullptr;
		glfwTerminate();

		clz::log::info("Window shutdown successful");
	}

	/// @copydoc
	void pollEventsGLFW(GLFWwindow** pWindow)
	{
		glfwPollEvents();

		if (glfwWindowShouldClose(*pWindow))
		{
			clz::state::setEngineState(
				clz::state::EngineState::Shutdown,
				"window poll events"
			);
		}
	}
} // namespace clz::window
