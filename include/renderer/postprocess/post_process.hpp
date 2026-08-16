#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer::post_process
{
	bool initializePostProcesses();
	void destroyPostProcesses();
	bool recreatePostProcesses();

	/// @brief Runs the full pre-tonemap -> tonemap -> post-tonemap chain.
	/// @note Leaves postTonemapImage in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
	/// The caller is responsible for copying/blitting postTonemapImage into
	/// its actual final destination (swapchain image, editor viewport image, etc.)
	void applyPostProcessing(VkCommandBuffer commandBuffer);

}