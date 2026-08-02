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
	const int width = clz::config::getInt("window", "width", 800);
	const int height = clz::config::getInt("window", "height", 600);
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
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	*pWindow = glfwCreateWindow(
		width,
		height,
		clz::config::getAppName().c_str(),
		nullptr,
		nullptr
	);
	if (!(*pWindow))
	{
		log::error("Could not create GLFW window");
		return false;
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
