#pragma once

#include <vulkan/vulkan.h>
#include <array>

namespace clz::renderer::post_process
{
	struct bloomImage
	{
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory memory;
		VkExtent2D extent;
	};
	inline constexpr VkFormat BLOOM_IMAGE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	inline uint8_t constexpr NUM_BLOOM_MIPS = 5;
	inline std::array<bloomImage, NUM_BLOOM_MIPS> bloomMips;
	inline bloomImage bloomedImage;
	inline VkSampler bloomSampler;
	enum BloomProcessBits : uint32_t
	{
		DOWNSAMPLE = 1 << 0,
		UPSAMPLE   = 1 << 1,
		DISABLE    = 1 << 2,
	};

	bool createBloomProcess();
	void destroyBloomProcess();
	void applyBloomProcess(VkCommandBuffer commandBuffer);

	inline bool Bloom = true;
	inline float bloomStrength = 0.05f;
	inline float filterRadius = 0.005;

	inline void disableBloom()
	{
		Bloom = false;
	}
	inline void enableBloom()
	{
		Bloom = true;
	}
	inline void setBloomStrength(const float strength)
	{
		bloomStrength = strength;
	}
	inline float getBloomStrength()
	{
		return bloomStrength;
	}

	inline void setFilterRadius(const float radius)
	{
		filterRadius = radius;
	}
	inline float getFilterRadius()
	{
		return filterRadius;
	}
}
