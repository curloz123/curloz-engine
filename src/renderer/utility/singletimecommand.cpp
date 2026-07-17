#include "renderer/utility/singletimecommand.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include <vulkan/vulkan_core.h>

namespace clz::renderer
{
	VkCommandBuffer startSingleTimeCommand()
	{
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = r_commandContext.commandPool;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(clz::renderer::r_deviceContext.device, &allocInfo, &commandBuffer) != VK_SUCCESS)
		{
			clz::log::error("Could not single time command buffer");
			return VK_NULL_HANDLE;
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}
	void submitSingleTimeCommand(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(clz::renderer::r_deviceContext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(clz::renderer::r_deviceContext.graphicsQueue);
	}

} // namespace clz::renderer
