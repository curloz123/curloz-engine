#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer::post_process
{
	struct bloomImage
	{
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory memory;
	};
	inline constexpr VkFormat BLOOM_IMAGE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	inline bloomImage horizontalBloomImage;
	inline bloomImage verticalBloomImage;
	inline bloomImage bloomBlendImage;
	inline VkSampler bloomSampler;
	inline bool enableBloom = true;
	enum BloomProcessBits : uint32_t
	{
		FIRST_TIME = 1 << 0,
		HORIZONTAL = 1 << 1,
		VERTICAL   = 1 << 2,
		BLEND      = 1 << 3,
		DISABLED   = 1 << 4
	};

	bool createBloomProcess();
	void destroyBloomProcess();
	void applyBloomProcess(VkCommandBuffer commandBuffer);
}
