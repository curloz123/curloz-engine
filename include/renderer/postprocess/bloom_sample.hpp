#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer::post_process
{
	inline constexpr VkFormat BLOOM_SAMPLE_IMAGE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	inline VkImage bloomSampleImage;
	inline VkImageView bloomSampleImageView;
	inline VkSampler bloomSampleSampler;
	inline VkDeviceMemory bloomSampleImageMemory;

	bool createBloomSampleProcess();
	void destroyBloomSampleProcess();
	void applyBloomSampleProcess(VkCommandBuffer commandBuffer);
}
