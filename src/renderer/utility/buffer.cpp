/**
 * @file buffer.cpp
 * @author curl0z
 *
 * @brief Implementation of buffer.hpp
 */

#include "renderer/utility/buffer.hpp"

#include "core/assert.hpp"
#include "core/logs.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	bool createBuffer(
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory,
		const std::string& bufferName,
		const VkDeviceSize size,
		const VkBufferUsageFlags usage,
		const VkMemoryPropertyFlags properties
	)
	{
		const auto& device = clz::renderer::r_deviceContext.device;
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			clz::log::error("vulkan failed to create buffer: " + bufferName);
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex =
			findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			clz::log::error("could not allocate memory for buffer: " + bufferName);
			clz::CLZ_ASSERT(false, "could not allocate memory");
			return false;
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
		return true;
	}

	bool copyBuffer(
		const VkBuffer srcBuffer,
		const VkBuffer dstBuffer,
		const VkDeviceSize size,
		const std::string& srcBufferName,
		const std::string& dstBufferName
	)
	{
		const auto& device = clz::renderer::r_deviceContext.device;
		const auto& commandPool = clz::renderer::r_commandContext.commandPool;
		const auto& graphicsQueue = clz::renderer::r_deviceContext.graphicsQueue;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
			[[unlikely]]
		{
			clz::log::error(
				"could not allocate single time command buffer while"
				" copying buffer: " +
				srcBufferName + " to " + dstBufferName
			);
			clz::CLZ_ASSERT(false, "could not allocate single time command buffer");
			return false;
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			clz::log::error(
				"vulkan could not"
				" copy buffer: " +
				srcBufferName + " to " + dstBufferName
			);
			clz::CLZ_ASSERT(false, "could not copy src to dst buffer");
			return false;
		}
		vkQueueWaitIdle(graphicsQueue);
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

		return true;
	}
} // namespace clz::renderer
