#include "core/logs.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"
#include "renderer/postprocess/bloom_sample.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/utility/memory.hpp"
#include <vulkan/vulkan_core.h>
#include "renderer/pipelinedata/post_process.hpp"

namespace clz::renderer::post_process
{
	bool createBloomSampleProcess()
	{
		if (!createImage(
			bloomSampleImage,
			    "bloom sample image",
			    r_renderTargetContext.imageExtent.width / 2,
			    r_renderTargetContext.imageExtent.height / 2,
			    BLOOM_SAMPLE_IMAGE_FORMAT,
			    VK_IMAGE_TILING_OPTIMAL,
			    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
			    	VK_IMAGE_USAGE_SAMPLED_BIT,
			    0
		    ))
		{
			clz::log::error("failed to create bloom sample image");
			return false;
		}

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(
			renderer::r_deviceContext.device,
			bloomSampleImage,
			&memReq
		);
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = renderer::findMemoryType(
			memReq.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		if (vkAllocateMemory(
			    renderer::r_deviceContext.device,
			    &allocInfo,
			    nullptr,
			    &bloomSampleImageMemory
		    ) != VK_SUCCESS)
		{
			clz::log::error("failed to allocate pre tonemap image");
			return false;
		}
		vkBindImageMemory(
			r_deviceContext.device,
			bloomSampleImage,
			bloomSampleImageMemory,
			0
		);
		setHandleName(
			reinterpret_cast<uint64_t>(bloomSampleImageMemory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			"bloom sampler memory"
		);

		if (!createImageView(
			bloomSampleImageView,
			"bloom sample image view",
			bloomSampleImage,
			BLOOM_SAMPLE_IMAGE_FORMAT,
			VK_IMAGE_ASPECT_COLOR_BIT))
		{
			clz::log::error("failed to create post tonemap image view");
			return false;
		}

		if (!createSampler(
				bloomSampleSampler,
				"bloom image sampler",
				VK_FILTER_LINEAR,
				VK_FILTER_LINEAR,
				VK_SAMPLER_MIPMAP_MODE_LINEAR,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
			     ))
		{
			clz::log::error("Could not create bloom sample image");
			return false;
		}

		clz::log::info("Created pre-tonemap post process resources");
		return true;
	}

	void destroyBloomSampleProcess()
	{
		vkDestroySampler(r_deviceContext.device, bloomSampleSampler, nullptr);
		vkDestroyImageView(r_deviceContext.device, bloomSampleImageView, nullptr);
		vkDestroyImage(r_deviceContext.device, bloomSampleImage, nullptr);
		vkFreeMemory(r_deviceContext.device, bloomSampleImageMemory, nullptr);
	}

	void applyBloomSampleProcess(VkCommandBuffer commandBuffer)
	{
		transition_image_layout(
			bloomSampleImage,
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
		colorAttachment.imageView = bloomSampleImageView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {
			.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
		};

		VkExtent2D extent;
		extent.width = r_renderTargetContext.imageExtent.width / 2;
		extent.height = r_renderTargetContext.imageExtent.height / 2;
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
			r_bloomSamplePipelineContext.pipeline);
		// const Pre_TonemapPC pushConstant{
		// 	.postProcessBits = 1,
		// };
		// vkCmdPushConstants(
		// 	commandBuffer,
		// 	r_bloomSamplePipelineContext.layout,
		// 	VK_SHADER_STAGE_VERTEX_BIT |
		// 	VK_SHADER_STAGE_FRAGMENT_BIT,
		// 	0,
		// 	sizeof(Pre_TonemapPC),
		// 	&pushConstant
		// );
		vkCmdBindDescriptorSets(
			    commandBuffer,
			    VK_PIPELINE_BIND_POINT_GRAPHICS,
			    r_bloomSamplePipelineContext.layout,
			    0, 1,
			    &post_processDescriptorSets[r_currentFrame],
			    0,
			    nullptr
		);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		vkCmdEndRendering(commandBuffer);

		// transition_image_layout(
		// 	bloomSampleImage,
		// 	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		// 	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		// 	VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		// 	VK_ACCESS_2_TRANSFER_READ_BIT,
		// 	VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		// 	VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		// 	VK_IMAGE_ASPECT_COLOR_BIT,
		// 	commandBuffer
		// );

		transition_image_layout(
			bloomSampleImage,
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
