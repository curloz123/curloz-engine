#include "renderer/postprocess/tonemap.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/pipelinedata/post_process.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/namer.hpp"

namespace clz::renderer::post_process
{
	bool createTonemapProcess()
	{
		if (!createImage(
			tonemapImage,
			    "tone-map image",
			    r_renderTargetContext.imageExtent.width,
			    r_renderTargetContext.imageExtent.height,
			    TONEMAP_IMAGE_FORMAT,
			    VK_IMAGE_TILING_OPTIMAL,
			    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			    0
		    ))
		{
			clz::log::error("failed to create tonemap image");
			return false;
		}

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(
			r_deviceContext.device,
			tonemapImage,
			&memReq
		);
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = findMemoryType(
			memReq.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		if (vkAllocateMemory(
			    r_deviceContext.device,
			    &allocInfo,
			    nullptr,
			    &tonemapMemory
		    ) != VK_SUCCESS)
		{
			clz::log::error("failed to allocate tonemap image");
			return false;
		}
		vkBindImageMemory(
			r_deviceContext.device,
			tonemapImage,
			tonemapMemory,
			0
		);

		if (!createImageView(
			tonemapImageView,
			"post tonemap image view",
			tonemapImage,
			TONEMAP_IMAGE_FORMAT,
			VK_IMAGE_ASPECT_COLOR_BIT))
		{
			clz::log::error("failed to create post tonemap image view");
			return false;
		}

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(
			    r_deviceContext.device,
			    &samplerInfo,
			    nullptr,
			    &tonemapSampler
		    ) != VK_SUCCESS)
		{
			clz::log::error("failed to create tonemap sampler");
			return false;
		}

		setHandleName(
			reinterpret_cast<uint64_t>(tonemapImage),
			VK_OBJECT_TYPE_IMAGE,
			"tonemap image"
		);
		setHandleName(
			reinterpret_cast<uint64_t>(tonemapImageView),
			VK_OBJECT_TYPE_IMAGE_VIEW,
			"tonemap image view"
		);
		setHandleName(
			reinterpret_cast<uint64_t>(tonemapSampler),
			VK_OBJECT_TYPE_SAMPLER,
			"tonemap sampler"
		);

		clz::log::info("Created tonemap process resources");
		return true;
	}

	void destroyTonemapProcess()
	{
		vkDestroySampler(r_deviceContext.device, tonemapSampler, nullptr);
		vkDestroyImageView(r_deviceContext.device, tonemapImageView, nullptr);
		vkDestroyImage(r_deviceContext.device, tonemapImage, nullptr);
		vkFreeMemory(r_deviceContext.device, tonemapMemory, nullptr);
	}

	void applyTonemapProcess(VkCommandBuffer commandBuffer)
	{
		transition_image_layout(
			tonemapImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			0,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			VK_IMAGE_ASPECT_COLOR_BIT,
			commandBuffer
		);
		VkRenderingAttachmentInfoKHR colorAttachment = {};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachment.pNext = nullptr;
		colorAttachment.imageView = tonemapImageView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {
			.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
		};

		const auto extent = r_renderTargetContext.imageExtent;
		VkRenderingInfoKHR renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderingInfo.pNext = nullptr;
		renderingInfo.flags = 0;
		renderingInfo.renderArea = {{0, 0}, extent};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = nullptr;

		vkCmdBeginRendering(commandBuffer, &renderingInfo);
		const VkViewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(extent.width),
			.height = static_cast<float>(extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		const VkRect2D scissor{{0, 0}, extent};
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			r_tonemapPipelineContext.pipeline);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			r_tonemapPipelineContext.layout,
			0,
			1,
			&post_processDescriptorSets[r_currentFrame],
			0,
			nullptr
			);
		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		vkCmdEndRendering(commandBuffer);

		transition_image_layout(
			tonemapImage,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			VK_ACCESS_2_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			commandBuffer
		);

	}

}