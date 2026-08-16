#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer::post_process
{
	inline constexpr VkFormat PRE_TONEMAP_IMAGE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	inline VkImage preTonemapImage;
	inline VkImageView preTonemapImageView;
	inline VkSampler preTonemapSampler;
	inline VkDeviceMemory preTonemapMemory;

	bool createPreTonemapProcess();
	void destroyPreTonemapProcess();
	void applyPreTonemapProcess(VkCommandBuffer commandBuffer);
}