/**
 * @file post_process.hpp
 * @author curl0z
 * @brief Post-process main header file
 * Every initialization, application, destruction is provided here
 */
#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer::post_process
{
	/// @brief Initializes all post-processes
	/// @return True on success, false otherwise
	bool initializePostProcesses();

	/// @brief Destroy's post processes resources
	void destroyPostProcesses();

	/// @brief Recreates post processes resources
	/// @return True on success, false otherwise
	bool recreatePostProcesses();

	/// @brief Runs the full ping-pong post_process chain
	/// @note Leaves postTonemapImage in VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
	/// The caller is responsible for copying/blitting postTonemapImage into
	/// its actual final destination (swapchain image, editor viewport image, etc.)
	void applyPostProcessing(VkCommandBuffer commandBuffer);

}
