#include "renderer/postprocess/post_process.hpp"

#include "core/assert.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/namer.hpp"

// assumes swapchain context has been initialized
namespace clz::renderer::post_process
{
	bool initializePostProcesses()
	{
		if (!createImage(
			postProcessImage,
			"main post processing image",
			r_swapchainContext.extent.width,
			r_swapchainContext.extent.height,
			postProcessImageFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			0))
		{
			clz::log::error("Failed to create post processing image");
			CLZ_ASSERT(false, "");
			return false;
		}

		setHandleName(
			reinterpret_cast<uint64_t>(postProcessImage),
			VK_OBJECT_TYPE_IMAGE,
			"post processing image");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(
			r_deviceContext.device,
			postProcessImage,
			&memRequirements);
		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memRequirements.size;
		memAllocInfo.memoryTypeIndex =
			findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(
			r_deviceContext.device,
			&memAllocInfo,
			nullptr,
			&postProcessImageMemory) != VK_SUCCESS)
		{
			clz::log::error("Failed to allocate post processing image memory");
			CLZ_ASSERT(false, "");
			return false;
		}
		vkBindImageMemory(
			r_deviceContext.device,
			postProcessImage,
			postProcessImageMemory,
			0);

		setHandleName(
			reinterpret_cast<uint64_t>(postProcessImageMemory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			"post processing image memory");

		if (!createImageView(
			postProcessImageView,
			"post processing image view",
			postProcessImage,
			postProcessImageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT
			))
		{
			clz::log::error("Failed to create post processing image view");
			CLZ_ASSERT(false, "");
			return false;
		}
		setHandleName(
			reinterpret_cast<uint64_t>(postProcessImageView),
			VK_OBJECT_TYPE_IMAGE_VIEW,
			"post processing image view");

		return true;
	}

	void applyPostProcessing()
	{

	}

	void destroyPostProcesses()
	{
		vkDestroyImageView(r_deviceContext.device, postProcessImageView, nullptr);
		vkDestroyImage(r_deviceContext.device, postProcessImage, nullptr);
		vkFreeMemory(r_deviceContext.device, postProcessImageMemory, nullptr);
	}
}
