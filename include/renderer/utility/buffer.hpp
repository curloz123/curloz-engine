/**
 * @file buffer.hpp
 * @author curl0z
 *
 * @brief Wraps all buffer related operations
 */
#pragma once

#include <string>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	/**
	 * @brief Creates a vulkan buffer
	 *
	 * @param buffer Buffer to create
	 * @param bufferMemory Buffer memory allocated to this given buffer
	 * @param bufferName Name of the buffer, required for debugging purposes
	 * @param size Size of the buffer memory to allocate
	 * @param usage Buffer usage flags
	 * @param properties Buffer properties
	 * @return True if creation was successful, false if not
	 */
	bool createBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const std::string& bufferName, VkDeviceSize size, VkBufferUsageFlags usage,
			  VkMemoryPropertyFlags properties);

	/**
	 * @brief Copies buffer from src to dst
	 *
	 * @param srcBuffer Source buffer
	 * @param dstBuffer Which buffer to copy data into
	 * @param size Size of the src buffer
	 * @param srcBufferName Src buffer name, required for debugging purposes
	 * @param dstBufferName Dst buffer name, required for debugging purposes
	 * @return True if creation was successful, false if not
	 */
	bool copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, const std::string& srcBufferName,
			const std::string& dstBufferName);
} // namespace clz::renderer