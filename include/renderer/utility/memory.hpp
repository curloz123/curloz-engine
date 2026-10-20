/**
 * @file memory.hpp
 * @author curl0z
 *
 * @brief Wraps all memory related functions
 */
#pragma once

#include <vulkan/vulkan.h>
#include "core/assert.hpp"

namespace clz::renderer
{
	/**
	 * @brief Find's suitable memory type as per given constraints
	 *
	 * @param typeFilter What filters to attach
	 * @param properties Which properties should the memory have
	 * @return Index of memory type
	 */
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(
			clz::renderer::r_deviceContext.physicalDevice,
			&memProperties
		);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
			    (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		clz::log::error("failed to find suitable memory type!");
		clz::CLZ_ASSERT(false, "could not find suitable memory type!");
		return UINT32_MAX;
	}
} // namespace clz::renderer
