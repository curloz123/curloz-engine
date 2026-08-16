#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer::post_process
{
	inline constexpr VkFormat TONEMAP_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
	inline VkImage tonemapImage;
	inline VkImageView tonemapImageView;
	inline VkSampler tonemapSampler;
	inline VkDeviceMemory tonemapMemory;

	bool createTonemapProcess();
	void destroyTonemapProcess();
	void applyTonemapProcess(VkCommandBuffer commandBuffer);
}