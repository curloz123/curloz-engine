#pragma once

#include <vulkan/vulkan.h>
#include "renderer/vk_types.hpp"
#include <optional>
#include <vulkan/vulkan_core.h>

namespace
{
	inline std::optional<float> maxAnisotropy;
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
}
