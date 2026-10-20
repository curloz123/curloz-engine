/**
 * @file render_target_context.hpp
 * @author curl0z
 * @brief render target context implementation file
 */

#include "renderer/context/render_target_context.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/utility/singletimecommand.hpp"
#include <vulkan/vulkan.h>
#include <string>
#include <span>
#include <expected>
#include <array>
#include "renderer/arena.hpp"

namespace clz::renderer
{
	/// @brief Creates render target
	/// @param width Width of render target's image
	/// @param height Height of render target's image
	/// @return True on succesful, false otherwise
	static bool createRenderTarget(uint32_t width, uint32_t height);

	/// @brief Destroy's render target's images.
	/// @param isRecreating Flag indicating whether resource is being recreated.
	/// false by default.
	static void destroyRenderTarget(const bool isRecreating = false);

	/// @brief Create's render target context's depth resources
	/// @return True on succesful, false otherwise
	static bool createDepthResources();

	/// @brief Destroy's render target context's depth resources
	/// @param isRecreating Flag indicating whether resource is being recreated.
	static void destroyDepthResources(const bool isRecreating = false);

}

namespace clz::renderer
{
	/// @copydoc initRenderTargetContext
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

	/// @copydoc destroyRenderTargetContext
	void destroyRenderTargetContext(const bool isRecreating)
	{
		destroyDepthResources(isRecreating);
		destroyRenderTarget(isRecreating);
	}
	
	/// @copydoc recreateRenderTargetContext
	bool recreateRenderTargetContext(const uint32_t width, const uint32_t height)
	{
		const bool isRecreating = true;
		destroyRenderTargetContext(isRecreating);
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
	/// @copydoc createRenderTarget
	bool createRenderTarget(const uint32_t width, const uint32_t height)
	{
		r_renderTargetContext.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		r_renderTargetContext.imageExtent.width = width;
		r_renderTargetContext.imageExtent.height = height;

		r_renderTargetContext.msaaFlagBits = MSAA;

		if (!createImage(
			r_renderTargetContext.image,
			"main render target image",
			width,
			height,
			r_renderTargetContext.imageFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			0)
		)
		{
			clz::log::error("vulkan failed to create render target image");
			return false;
		}
		if (!createImage(
			r_renderTargetContext.msaaImage,
			"main render target msaa image",
			width,
			height,
			r_renderTargetContext.imageFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			0,
			1,
			r_renderTargetContext.msaaFlagBits)
		)
		{
			clz::log::error("vulkan failed to create render target msaa image");
			return false;
		}

		VkMemoryRequirements memRequirements;
		VkMemoryRequirements msaaMemRequirements;

		vkGetImageMemoryRequirements(
			r_deviceContext.device,
			r_renderTargetContext.image,
			&memRequirements
		);
		vkGetImageMemoryRequirements(
			r_deviceContext.device,
			r_renderTargetContext.msaaImage,
			&msaaMemRequirements
		);

		ImageMemoryAllocInfo allocInfo{
			.imageType = ImageType::CANVAS,
			.alignment = memRequirements.alignment,
			.size = memRequirements.size
		};
		ImageMemoryAllocInfo msaaAllocInfo{
			.imageType = ImageType::MULTISAMPLED,
			.alignment = msaaMemRequirements.alignment,
			.size = msaaMemRequirements.size
		};

		if (!carveImageMemory(allocInfo, r_renderTargetContext.imageMemory))
			clz::log::error("Could not carve memory for render target context", true);
		if (!carveImageMemory(msaaAllocInfo, r_renderTargetContext.msaaImageMemory))
			clz::log::error("Could not carve memory for render target context", true);

		bindImageMemory(
			r_renderTargetContext.image,
			r_renderTargetContext.imageMemory
		);
		bindImageMemory(
			r_renderTargetContext.msaaImage,
			r_renderTargetContext.msaaImageMemory
		);

		if (!createImageView(
			r_renderTargetContext.imageView,
			"render target context image view",
			r_renderTargetContext.image,
			r_renderTargetContext.imageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT
			))
		{
			clz::log::error("vulkan failed to create render target image view");
			return false;
		}
		if (!createImageView(
				r_renderTargetContext.msaaImageView,
				"render target context's msaa image view",
				r_renderTargetContext.msaaImage,
				r_renderTargetContext.imageFormat,
				VK_IMAGE_ASPECT_COLOR_BIT
			)
		)
		{
			clz::log::error("vulkan failed to create render target msaa image view");
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

		if (!createSampler(
			r_renderTargetContext.msaaImageSampler,
			"render target sampler",
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
			))
		{
			clz::log::error("Could not create render target msaa sampler");
			return false;
		}

		return true;
	}

	/// @copydoc destroyRenderTarget
	static void destroyRenderTarget(const bool isRecreating)
	{
		vkDestroySampler(r_deviceContext.device, r_renderTargetContext.imageSampler, nullptr);
		vkDestroySampler(r_deviceContext.device, r_renderTargetContext.msaaImageSampler, nullptr);
		vkDestroyImageView(r_deviceContext.device, r_renderTargetContext.imageView, nullptr);
		vkDestroyImageView(r_deviceContext.device, r_renderTargetContext.msaaImageView, nullptr);
		vkDestroyImage(r_deviceContext.device, r_renderTargetContext.image, nullptr);
		vkDestroyImage(r_deviceContext.device, r_renderTargetContext.msaaImage, nullptr);

		if (isRecreating)
		{
			freeImageMemory(r_renderTargetContext.imageMemory);
			freeImageMemory(r_renderTargetContext.msaaImageMemory);
		}
	}

	/// @copydoc createDepthResources
	bool createDepthResources()
	{
		r_renderTargetContext.depthFormat = VK_FORMAT_D32_SFLOAT;

		if (!createImage(
			    r_renderTargetContext.depthImage,
			    "render target depth Image",
			    r_renderTargetContext.imageExtent.width,
			    r_renderTargetContext.imageExtent.height,
			    r_renderTargetContext.depthFormat,
			    VK_IMAGE_TILING_OPTIMAL,
			    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			    0,
			    1,
			    r_renderTargetContext.msaaFlagBits
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
		ImageMemoryAllocInfo memAllocInfo{
			.imageType = ImageType::DEPTH,
			.alignment = memRequirements.alignment,
			.size = memRequirements.size
		};
		carveImageMemory(memAllocInfo, r_renderTargetContext.depthImageMemory);
		bindImageMemory(
			r_renderTargetContext.depthImage,
			r_renderTargetContext.depthImageMemory
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

	/// @copydoc destroyDepthResources
	static void destroyDepthResources(const bool isRecreating)
	{
		vkDestroyImageView(
			r_deviceContext.device,
			r_renderTargetContext.depthImageView,
			nullptr
		);
		vkDestroyImage(
			r_deviceContext.device,
			r_renderTargetContext.depthImage,
			nullptr
		);
		if (isRecreating)
			freeImageMemory(r_renderTargetContext.depthImageMemory);
	}
}
