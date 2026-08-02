/**
 * @file vulkanhelper.cpp
 * @author curl0z
 * @brief Implementation of all the vulkan helper functions
 */

#include "window/vulkanhelper.hpp"
#include "core/logs.hpp"
#include "renderer/renderer.hpp"
#include "window/window_types.hpp"

namespace clz::window
{
/// @copydoc
bool getRequiredVulkanExtensions(std::vector<const char*>& rRequiredExtensions)
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	if (glfwExtensionCount == 0 || glfwExtensions == nullptr)
	{
		log::error("Could not retrieve extensions required for instance by window");
		return false;
	}
	rRequiredExtensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);
	return true;
}

/// @copydoc
bool createVulkanSurface(VkInstance instance, VkSurfaceKHR& rSurface)
{
	if (glfwCreateWindowSurface(instance, w_window, nullptr, &rSurface) != VK_SUCCESS)
	{
		log::error("Could not create window surface");
		return false;
	}

	return true;
}

/// @copydoc
void hintRendererAboutResize(GLFWwindow* window, int, int)
{
	clz::renderer::r_recreateSwapchain = true;
}

} // namespace clz::window
