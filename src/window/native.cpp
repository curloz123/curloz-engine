/**
 * @file native.cpp
 * @author curl0z
 * @brief implements all the internal GLFW functions
 */

#include "window/native.hpp"
#include "config/config.hpp"
#include "core/enginestate.hpp"
#include "core/logs.hpp"

namespace clz::window
{
	/// @copydoc
	bool initializeGLFW(GLFWwindow** pWindow)
	{
		const int width = clz::config::getValue<int>("window", "width", 800);
		const int height = clz::config::getValue<int>("window", "height", 600);

		if (width < 0 || height < 0)
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
		const bool fullscreen = clz::config::getValue<bool>("window", "fullscreen", false);
		if (fullscreen)
		{
			monitor = glfwGetPrimaryMonitor();
		}

		/// --- create window finally --- ///
		*pWindow = glfwCreateWindow(
			1920,
			1080,
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
		const bool enableRawInput =
			clz::config::getValue<bool>("window", "enable_raw_input", true);
		if (glfwRawMouseMotionSupported() && enableRawInput)
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
