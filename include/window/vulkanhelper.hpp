/**
 * @file vulkanhelper.hpp
 * @brief Public header to help vulkan
 */
#pragma once

#define GLFW_INCLUDE_VULKAN
// GLFW's Vulkan define only skips the platform Vulkan headers, not the
// legacy OpenGL header block further down in glfw3.h on Linux/X11. This
// engine never uses OpenGL, so also opt out of that block explicitly
// rather than depending on system OpenGL headers being installed.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::window
{
/**
 * @brief Returns all required vulkan extensions to interact with window
 * @param rRequiredExtensions All vector where all extensions will be stored
 * @return void if function was a success
 * @return string as error about what went wrong
 */
bool getRequiredVulkanExtensions(std::vector<const char*>& rRequiredExtensions);

/**
 * @brief Creates vulkan surface
 * @param instance Vulkan instance
 * @param rSurface reference of surface to be created
 * @return void if function was a success
 * @return string as error about what went wrong
 */
bool createVulkanSurface(VkInstance instance, VkSurfaceKHR& rSurface);

/**
 * @brief Callback function that is called
 * whenever window's framebuffer extents are changed.
 * Hints renderer about this change too
 */
void hintRendererAboutResize(GLFWwindow* window, int, int);
} // namespace clz::window
