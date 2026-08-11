/**
 * @file memory.hpp
 * @author curl0z
 *
 * @brief Wraps all memory related functions
 */
#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer
{
	/**
	 * @brief Find's suitable memory type as per given constraints
	 *
	 * @param typeFilter What filters to attach
	 * @param properties Which properties should the memory have
	 * @return Index of memory type
	 */
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
} // namespace clz::renderer