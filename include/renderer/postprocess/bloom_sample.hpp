#pragma once

#include <vulkan/vulkan.h>
#include "../arena.hpp"

namespace clz::renderer::post_process
{
	inline constexpr VkFormat BLOOM_SAMPLE_IMAGE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	inline VkImage bloomSampleImage;
	inline VkImageView bloomSampleImageView;
	inline VkSampler bloomSampleSampler;
	inline clzImageMemory bloomSampleImageMemory;

	/**
	 * @brief Creates bloom sample process
	 * @param width New width of post_process images
	 * @param height New height of post_process images
	 * @return True on success, false otherwise
	 */
	bool createBloomSampleProcess(uint32_t width, uint32_t height);
	void destroyBloomSampleProcess(const bool isRecreating = false);
	void applyBloomSampleProcess(VkCommandBuffer commandBuffer);
}
