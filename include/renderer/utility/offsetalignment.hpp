/**
 * @file offsetalignment.hpp
 * @author curl0z
 *
 * @brief Contains all funtions regarding vulkan's alignment and all those quirks
 */
#pragma once

#include "renderer/vk_types.hpp"
#include <optional>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	inline std::optional<VkDeviceSize> UBOOffsetAlignment;
	inline std::optional<VkDeviceSize> SSBOOffsetAlignment;

	inline VkDeviceSize getNextUBOOffsetAlignment(const VkDeviceSize prvOffset)
	{
		if (!UBOOffsetAlignment.has_value())
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(r_deviceContext.physicalDevice, &properties);
			UBOOffsetAlignment = properties.limits.minUniformBufferOffsetAlignment;
		}

		return (UBOOffsetAlignment.value() + prvOffset - 1) &
		       ~(UBOOffsetAlignment.value() - 1);
	}
	inline VkDeviceSize getNextSSBOOffsetAlignment(const VkDeviceSize prvOffset)
	{
		if (!SSBOOffsetAlignment.has_value())
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(r_deviceContext.physicalDevice, &properties);
			SSBOOffsetAlignment = properties.limits.minStorageBufferOffsetAlignment;
		}
		return (SSBOOffsetAlignment.value() + prvOffset - 1) &
		       ~(SSBOOffsetAlignment.value() - 1);
	}

	inline VkDeviceSize nextImageOffset(const VkDeviceSize imageSize, const VkDeviceSize nextImageAlignment)
	{
		return (imageSize + nextImageAlignment - 1) & ~(nextImageAlignment - 1);
	}
} // namespace clz::renderer
