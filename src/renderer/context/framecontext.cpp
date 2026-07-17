/**
 * @file framecontext.cpp
 * @author curl0z
 * @brief Implementation of the initialization
 * and cleanup of frame context
 */
#include "renderer/context/framecontext.hpp"
#include "core/logs.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	bool initFrameContext()
	{
		if (!createSyncObjects())
		{
			clz::log::error("Could not create sync objects");
			clz::log::error("Could not initialize frame context");
			return false;
		}

		return true;
	}

	bool createSyncObjects()
	{
		r_frameContext.renderReadySemaphores.resize(r_FRAMES_IN_FLIGHT);
		r_frameContext.inFlightFences.resize(r_FRAMES_IN_FLIGHT);
		for (auto i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			VkSemaphoreCreateInfo semaphoreInfo{
			    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			};
			if (vkCreateSemaphore(r_deviceContext.device, &semaphoreInfo, nullptr, &r_frameContext.renderReadySemaphores[i]) !=
			    VK_SUCCESS)
			{
				clz::log::error("vulkan could not create semaphores");
				return false;
			}
			std::string semaphoreName = "renderer-ready-semaphore_" + std::to_string(i);
			setHandleName(reinterpret_cast<uint64_t>(r_frameContext.renderReadySemaphores[i]), VK_OBJECT_TYPE_SEMAPHORE,
				      semaphoreName.c_str());

			VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
			if (vkCreateFence(r_deviceContext.device, &fenceInfo, nullptr, &r_frameContext.inFlightFences[i]) != VK_SUCCESS)
			{
				clz::log::error("vulkan could not create fence");
				return false;
			}
			std::string fenceName = "fence_" + std::to_string(i);
			setHandleName(reinterpret_cast<uint64_t>(r_frameContext.inFlightFences[i]), VK_OBJECT_TYPE_FENCE, fenceName.c_str());
		}

		r_frameContext.presentReadySemaphores.resize(r_swapchainContext.images.size());
		for (auto i = 0; i < r_swapchainContext.images.size(); ++i)
		{
			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			if (vkCreateSemaphore(r_deviceContext.device, &semaphoreInfo, nullptr, &r_frameContext.presentReadySemaphores[i]) !=
			    VK_SUCCESS)
			{
				clz::log::error("vulkan Could not create semaphores");
				return false;
			}

			std::string semaphoreName = "renderer-ready-semaphore_" + std::to_string(i);
			setHandleName(reinterpret_cast<uint64_t>(r_frameContext.presentReadySemaphores[i]), VK_OBJECT_TYPE_SEMAPHORE,
				      semaphoreName.c_str());
		}

		clz::log::info("created All semaphores and fences");
		return true;
	}
} // namespace clz::renderer

namespace clz::renderer
{
	void destroyFrameContext()
	{
		destroySyncObjects();

		clz::log::info("destroyed frame context");
	}
	void destroySyncObjects()
	{
		for (const auto semaphore : r_frameContext.renderReadySemaphores)
		{
			vkDestroySemaphore(r_deviceContext.device, semaphore, nullptr);
		}

		for (const auto inFlightFence : r_frameContext.inFlightFences)
		{
			vkDestroyFence(r_deviceContext.device, inFlightFence, nullptr);
		}

		for (const auto semaphore : r_frameContext.presentReadySemaphores)
		{
			vkDestroySemaphore(r_deviceContext.device, semaphore, nullptr);
		}

		clz::log::info("destroyed semaphores and fences");
	}

} // namespace clz::renderer
