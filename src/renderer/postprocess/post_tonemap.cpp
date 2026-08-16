#include "renderer/postprocess/post_tonemap.hpp"

#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/pipelinedata/post_process.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/namer.hpp"

namespace clz::renderer::post_process
{
	bool createPostTonemapProcess()
	{
		if (!createImage(
			postTonemapImage,
			    "post tone-map image",
			    r_renderTargetContext.imageExtent.width,
			    r_renderTargetContext.imageExtent.height,
			    POST_TONEMAP_IMAGE_FORMAT,
			    VK_IMAGE_TILING_OPTIMAL,
			    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			    0
		    ))
		{
			clz::log::error("failed to create post tonemap image");
			return false;
		}

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(
			r_deviceContext.device,
			postTonemapImage,
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
			    &postTonemapMemory
		    ) != VK_SUCCESS)
		{
			clz::log::error("failed to allocate post tonemap image");
			return false;
		}
		vkBindImageMemory(
			r_deviceContext.device,
			postTonemapImage,
			postTonemapMemory,
			0
		);

		// --- Image view ---
		if (!createImageView(
			postTonemapImageView,
			"post tonemap image view",
			postTonemapImage,
			POST_TONEMAP_IMAGE_FORMAT,
			VK_IMAGE_ASPECT_COLOR_BIT))
		{
			clz::log::error("failed to create post tonemap image view");
			return false;
		}

		// --- Linear sampler ---
		if (!createSampler(
			postTonemapSampler,
			"post tonemap sampler",
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
			)
		)
		{
			clz::log::error("Could not create post tonemap sampler");
			return false;
		}

		clz::log::info("Created post-tonemap post process resources");
		return true;
	}

	void destroyPostTonemapProcess()
	{
		vkDestroySampler(r_deviceContext.device, postTonemapSampler, nullptr);
		vkDestroyImageView(r_deviceContext.device, postTonemapImageView, nullptr);
		vkDestroyImage(r_deviceContext.device, postTonemapImage, nullptr);
		vkFreeMemory(r_deviceContext.device, postTonemapMemory, nullptr);
	}

	void applyPostTonemapProcess(VkCommandBuffer commandBuffer)
	{
		transition_image_layout(
			postTonemapImage,
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
		colorAttachment.imageView = postTonemapImageView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {
			.color = {
				.float32 = {0.0f, 0.0f, 0.0f, 1.0f}
			}
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
			r_postTonemapPipelineContext.pipeline);
		const Post_TonemapPC pushConstant{
			.postProcessBits = 1,
		};
		vkCmdPushConstants(
			commandBuffer,
			r_postTonemapPipelineContext.layout,
			VK_SHADER_STAGE_VERTEX_BIT |
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(Post_TonemapPC),
			&pushConstant
		);
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			r_postTonemapPipelineContext.layout,
			0, 1,
			&post_processDescriptorSets[r_currentFrame],
			0,
			nullptr
		);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		vkCmdEndRendering(commandBuffer);

		// Leave in TRANSFER_SRC_OPTIMAL: the caller copies/blits this into
		// its real destination (swapchain image or editor viewport image).
		// Those destinations have their own formats that can differ from
		// POST_TONEMAP_IMAGE_FORMAT, so this pipeline can't render directly
		// into them -- that mismatch was the cause of the GPU crash.
		transition_image_layout(
			postTonemapImage,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
			VK_IMAGE_ASPECT_COLOR_BIT,
			commandBuffer
		);
	}
}