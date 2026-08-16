#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer::post_process
{
	inline constexpr VkFormat POST_TONEMAP_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
	inline VkImage postTonemapImage;
	inline VkImageView postTonemapImageView;
	inline VkSampler postTonemapSampler;
	inline VkDeviceMemory postTonemapMemory;

	bool createPostTonemapProcess();
	void destroyPostTonemapProcess();

	/// @brief Runs the post-tonemap pass, writing into postTonemapImage.
	/// @note Leaves postTonemapImage in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	/// ready to be copied/blitted into the swapchain or editor viewport image.
	void applyPostTonemapProcess(VkCommandBuffer commandBuffer);
}