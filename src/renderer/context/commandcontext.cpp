/**
 * @file commandcontext.cpp
 * @author curl0z
 * @brief Implementation of commandcontext.hpp
 */

#include "renderer/context/commandcontext.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	bool createCommandPool();
	bool createCommandBuffers();
	void destroyCommandPool();
} // namespace clz::renderer

namespace clz::renderer
{
	bool initCommandContext()
	{
		if (!createCommandPool())
		{
			clz::log::error("Could not initialize command context");
			return false;
		}

		if (!createCommandBuffers())
		{
			clz::log::error("Could not initialize command context");
			return false;
		}

		clz::log::info("Initialized command context");
		return true;
	}

	bool createCommandPool()
	{
		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = r_deviceContext.graphicsFamily.value();
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(r_deviceContext.device, &commandPoolInfo, nullptr, &r_commandContext.commandPool) != VK_SUCCESS)
		{
			clz::log::error("vulkan could not create command pool :(");
			return false;
		}

		clz::log::info("created command pool");
		return true;
	}

	bool createCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = r_commandContext.commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = r_FRAMES_IN_FLIGHT;

		r_commandContext.commandBuffer.resize((r_FRAMES_IN_FLIGHT));

		if (vkAllocateCommandBuffers(r_deviceContext.device, &allocateInfo, r_commandContext.commandBuffer.data()) != VK_SUCCESS)
		{
			clz::log::error("Vulkan failed to allocate command buffers");
			return false;
		}

		clz::log::info("created command buffers");

		return true;
	}
} // namespace clz::renderer

namespace clz::renderer
{
	void destroyCommandContext()
	{
		destroyCommandPool();
	}

	void destroyCommandPool()
	{
		vkDestroyCommandPool(r_deviceContext.device, r_commandContext.commandPool, nullptr);
	}
} // namespace clz::renderer
