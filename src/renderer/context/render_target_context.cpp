#include "renderer/context/render_target_context.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/singletimecommand.hpp"
#include <vulkan/vulkan.h>
#include <string>
#include <span>
#include <expected>
#include <array>
#include <vulkan/vulkan_core.h>

namespace clz::renderer
{
	static bool createRenderTarget(uint32_t width, uint32_t height);
	static void destroyRenderTarget();

	static bool createDepthResources();
	static void destroyDepthResources();

}

namespace clz::renderer
{
	bool initRenderTargetContext(const uint32_t width, const uint32_t height)
	{
		if (!createRenderTarget(width, height))
		{
			clz::log::error("Unable to create render target");
			goto failure;
		}

		if (!createDepthResources())
		{
			clz::log::error("Unable to create render target's depth resources");
			goto failure;
		}

		clz::log::info("Initialized render target context");
		return true;

		failure:
		clz::log::error("Failed to initialize target context");
		return false;
	}

	void destroyRenderTargetContext()
	{
		destroyDepthResources();
		destroyRenderTarget();
	}

	bool recreateRenderTargetContext(const uint32_t width, const uint32_t height)
	{
		clz::log::debug("recreating render target with: "
				"width: " + std::to_string(width) + " "
				"height: " + std::to_string(height));
		vkDeviceWaitIdle(r_deviceContext.device);
		destroyRenderTargetContext();
		if (!initRenderTargetContext(width, height))
		{
			clz::log::error("failed to re-create render target context");
			return false;
		}
		return true;
	}
}

namespace clz::renderer
{
	bool createRenderTarget(const uint32_t width, const uint32_t height)
	{
		r_renderTargetContext.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		r_renderTargetContext.imageExtent.width = width;
		r_renderTargetContext.imageExtent.height = height;
		if (!createImage(
			r_renderTargetContext.image,
			"main post processing image",
			width,
			height,
			r_renderTargetContext.imageFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			0))
		{
			clz::log::error("vulkan failed to create render target image");
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(
			r_deviceContext.device,
			r_renderTargetContext.image,
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
			&r_renderTargetContext.imageMemory) != VK_SUCCESS)
		{
			clz::log::error("vulkan failed to allocate render target image memory");
			return false;
		}
		vkBindImageMemory(
			r_deviceContext.device,
			r_renderTargetContext.image,
			r_renderTargetContext.imageMemory,
			0);

		setHandleName(
			reinterpret_cast<uint64_t>(r_renderTargetContext.imageMemory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			"render target image memory");

		if (!createImageView(
			r_renderTargetContext.imageView,
			"post processing image view",
			r_renderTargetContext.image,
			r_renderTargetContext.imageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT
			))
		{
			clz::log::error("vulkan failed to create render target image view");
			return false;
		}

		if (!createSampler(
			r_renderTargetContext.imageSampler,
			"render target sampler",
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
			))
		{
			clz::log::error("Could not create render target sampler");
			return false;
		}

		return true;
	}

	void destroyRenderTarget()
	{
		vkDestroySampler(r_deviceContext.device, r_renderTargetContext.imageSampler, nullptr);
		vkDestroyImageView(r_deviceContext.device, r_renderTargetContext.imageView, nullptr);
		vkDestroyImage(r_deviceContext.device, r_renderTargetContext.image, nullptr);
		vkFreeMemory(r_deviceContext.device, r_renderTargetContext.imageMemory, nullptr);
	}

	bool createDepthResources()
	{
		std::array<VkFormat, 1> formatCandidates = {VK_FORMAT_D32_SFLOAT};
		const auto depthResult = findSupportedFormat(
			formatCandidates,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
		if (!depthResult)
		{
			clz::log::error(depthResult.error());
			return false;
		}
		r_renderTargetContext.depthFormat = depthResult.value();

		if (!createImage(
			    r_renderTargetContext.depthImage,
			    "render target depth Image",
			    r_renderTargetContext.imageExtent.width,
			    r_renderTargetContext.imageExtent.height,
			    r_renderTargetContext.depthFormat,
			    VK_IMAGE_TILING_OPTIMAL,
			    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			    0
		    ))
		{
			clz::log::error("Could not create render target depth image");
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(
			clz::renderer::r_deviceContext.device,
			r_renderTargetContext.depthImage,
			&memRequirements
		);

		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memRequirements.size;
		memAllocInfo.memoryTypeIndex = findMemoryType(
			memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		if (vkAllocateMemory(
			    clz::renderer::r_deviceContext.device,
			    &memAllocInfo,
			    nullptr,
			    &r_renderTargetContext.depthImageMemory
		    ) != VK_SUCCESS)
		{
			clz::log::error("vulkan could not create render target depth memory");
			return false;
		}
		setHandleName(
			reinterpret_cast<uint64_t>(r_renderTargetContext.depthImageMemory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			"render target depth memory"
		);

		vkBindImageMemory(
			clz::renderer::r_deviceContext.device,
			r_renderTargetContext.depthImage,
			r_renderTargetContext.depthImageMemory,
			0
		);

		if (!createImageView(
			    r_renderTargetContext.depthImageView,
			    "render target depth image view",
			    r_renderTargetContext.depthImage,
			    r_renderTargetContext.depthFormat,
			    VK_IMAGE_ASPECT_DEPTH_BIT
		    ))
		{
			clz::log::error("Could not create depth image view");
			return false;
		}
		setHandleName(
			reinterpret_cast<uint64_t>(r_renderTargetContext.depthImageView),
			VK_OBJECT_TYPE_IMAGE_VIEW,
			"render target depth image view"
		);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = clz::renderer::r_commandContext.commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer = startSingleTimeCommand();

		transition_image_layout(
			r_renderTargetContext.depthImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_NONE,
			VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
				VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_NONE,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			commandBuffer
		);

		submitSingleTimeCommand(commandBuffer);

		clz::log::info("Created render depth resources");
		return true;
	}
	void destroyDepthResources()
	{
		vkDestroyImageView(r_deviceContext.device, r_renderTargetContext.depthImageView, nullptr);
		vkDestroyImage(r_deviceContext.device, r_renderTargetContext.depthImage, nullptr);
		vkFreeMemory(r_deviceContext.device, r_renderTargetContext.depthImageMemory, nullptr);
	}
}
