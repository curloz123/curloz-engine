/**
 * @file memory.cpp
 * @author curl0z
 *
 * @brief Implementation of memory.hpp
 */

#include "renderer/utility/memory.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	uint32_t findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(clz::renderer::r_deviceContext.physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		clz::log::error("failed to find suitable memory type!");
		clz::CLZ_ASSERT(false, "could not find suitable memory type!");
		return UINT32_MAX;
	}
} // namespace clz::renderer