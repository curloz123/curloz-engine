#pragma once

#include <vulkan/vulkan.h>
#include "renderer/vk_types.hpp"
#include <optional>
#include <vulkan/vulkan_core.h>
#include "renderer/vk_types.hpp"
#include <bit>
#include "core/logs.hpp"

namespace
{
	inline std::optional<float> maxAnisotropy;
	inline std::optional<VkSampleCountFlagBits> maxMsaaSamples;
}

namespace clz::renderer
{
	inline float getMaxAnisotropy()
	{
		if (!maxAnisotropy.has_value())
		{
			VkPhysicalDeviceProperties2 properties = {};
			vkGetPhysicalDeviceProperties2(r_deviceContext.physicalDevice, &properties);
			maxAnisotropy = properties.properties.limits.maxSamplerAnisotropy;
		}

		return maxAnisotropy.value();
	}

	inline VkSampleCountFlagBits getMaxMsaaSamples()
	{
		if (!maxMsaaSamples.has_value())
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;
    			vkGetPhysicalDeviceProperties(
					r_deviceContext.physicalDevice, 
					&physicalDeviceProperties);

			VkSampleCountFlags counts = 
				physicalDeviceProperties.limits.framebufferColorSampleCounts & 
							physicalDeviceProperties.limits.framebufferDepthSampleCounts;

			if (counts & VK_SAMPLE_COUNT_64_BIT)
				maxMsaaSamples = VK_SAMPLE_COUNT_64_BIT;
			else if (counts & VK_SAMPLE_COUNT_32_BIT)
				maxMsaaSamples = VK_SAMPLE_COUNT_32_BIT;
			else if (counts & VK_SAMPLE_COUNT_16_BIT)
				maxMsaaSamples = VK_SAMPLE_COUNT_16_BIT;
			else if (counts & VK_SAMPLE_COUNT_8_BIT)
				maxMsaaSamples = VK_SAMPLE_COUNT_8_BIT;
			else if (counts & VK_SAMPLE_COUNT_4_BIT)
				maxMsaaSamples = VK_SAMPLE_COUNT_4_BIT;
			else if (counts & VK_SAMPLE_COUNT_2_BIT)
				maxMsaaSamples = VK_SAMPLE_COUNT_2_BIT;
			else
				maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
		}
		return maxMsaaSamples.value();
	}
	inline VkSampleCountFlagBits getMsaaFlagBitsFromInt(uint32_t flagInt)
	{
		flagInt = std::bit_floor(flagInt);
		switch(flagInt)
		{
		case(1):
			return VK_SAMPLE_COUNT_1_BIT;
			break;
		case(2):
			return VK_SAMPLE_COUNT_2_BIT;
			break;
		case(4):
			return VK_SAMPLE_COUNT_4_BIT;
			break;
		case(8):
			return VK_SAMPLE_COUNT_8_BIT;
			break;
		case(16):
			return VK_SAMPLE_COUNT_16_BIT;
			break;
		case(32):
			return VK_SAMPLE_COUNT_32_BIT;
			break;
		default:
			clz::log::warn("Vulkan Msaa invalid flag bits enquired, returning 1 bit flag");
		}

		return VK_SAMPLE_COUNT_1_BIT;
	}
}
