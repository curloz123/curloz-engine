#include "renderer/postprocess/bloom.hpp"
#include "renderer/pipelinedata/post_process.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"
#include "renderer/vk_types.hpp"
#include <array>
#include <string>
#include <vulkan/vulkan_core.h>
#include "renderer/utility/image.hpp"
#include "core/logs.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/namer.hpp"

namespace clz::renderer::post_process
{
	bool createBloomProcess()
	{
		std::array<bloomImage*, 3> images = {
			&horizontalBloomImage,
			&verticalBloomImage,
			&bloomBlendImage
		};
		const std::array<VkExtent2D, 3> extent = {{
			{
				r_renderTargetContext.imageExtent.width / 2,
				r_renderTargetContext.imageExtent.height / 2,
			},
			{
				r_renderTargetContext.imageExtent.width / 2,
				r_renderTargetContext.imageExtent.height / 2,
			},
			{
				r_renderTargetContext.imageExtent.width,
				r_renderTargetContext.imageExtent.height,
			}
		}
		};

		for (uint8_t i = 0; i < 2; ++i)
		{
			if (!createImage(
				images[i]->image,
				"bloom image: " + std::to_string(i),
				extent[i].width,
				extent[i].height,
				BLOOM_IMAGE_FORMAT,
				VK_IMAGE_TILING_OPTIMAL,
			    	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
			    		VK_IMAGE_USAGE_SAMPLED_BIT | 
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // delete this
				0
			))
			{
				clz::log::error("failed to create bloom image: " + std::to_string(i));
				return false;
			}

			VkMemoryRequirements memReq;
			vkGetImageMemoryRequirements(
				renderer::r_deviceContext.device,
				images[i]->image,
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
				    &images[i]->memory
			    ) != VK_SUCCESS)
			{
				clz::log::error("failed to allocate pre tonemap image");
				return false;
			}
			vkBindImageMemory(
				r_deviceContext.device,
				images[i]->image,
				images[i]->memory,
				0
			);
			setHandleName(
				reinterpret_cast<uint64_t>(images[i]->memory),
				VK_OBJECT_TYPE_DEVICE_MEMORY,
				"bloom memory"
			);
			
			if (!createImageView(
					images[i]->imageView,
					"bloom image view",
					images[i]->image,
					BLOOM_IMAGE_FORMAT,
					VK_IMAGE_ASPECT_COLOR_BIT)
			)
			{
				clz::log::error("failed to create bloom image view: " + std::to_string(i));
				return false;
			}
		}

		if (!createSampler(
			bloomSampler,
			"bloom sampler",
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
			)
		)
		{
			clz::log::error("could not create bloom image sampler");
			return false;
		}

		clz::log::info("Created bloom post-process");
		return true;
	}

	void destroyBloomProcess()
	{
		vkDestroySampler(r_deviceContext.device, bloomSampler, nullptr);
		vkDestroyImageView(r_deviceContext.device, verticalBloomImage.imageView, nullptr);
		vkDestroyImageView(r_deviceContext.device, horizontalBloomImage.imageView, nullptr);
		vkDestroyImage(r_deviceContext.device, verticalBloomImage.image, nullptr);
		vkDestroyImage(r_deviceContext.device, horizontalBloomImage.image, nullptr);
		vkFreeMemory(r_deviceContext.device, verticalBloomImage.memory, nullptr);
		vkFreeMemory(r_deviceContext.device, horizontalBloomImage.memory, nullptr);
	}

	void applyBloomProcess(VkCommandBuffer commandBuffer)
	{
		constexpr int amount = 10;
		bool horizontal = false;
		bool first_iteration = true;
		BloomPC pushConstant;
		
		for (int i = 0; i <= amount; ++i)
		{
			VkImage attachment;
			VkImageView attachmentView;
			VkExtent2D extent = r_renderTargetContext.imageExtent;

			if (i == amount)
			{
				attachment = horizontalBloomImage.image;
				attachmentView = horizontalBloomImage.imageView;
				pushConstant.bloomBits = BloomProcessBits::BLEND;
				extent.width = r_renderTargetContext.imageExtent.width / 2;
				extent.height = r_renderTargetContext.imageExtent.height / 2;

			}
			else if (first_iteration)
			{
				attachment = horizontalBloomImage.image;
				attachmentView = horizontalBloomImage.imageView;

				pushConstant.bloomBits = BloomProcessBits::FIRST_TIME;
				first_iteration = false;
				extent.width = r_renderTargetContext.imageExtent.width / 2;
				extent.height = r_renderTargetContext.imageExtent.height / 2;
			}
			else if (horizontal)
			{
				attachment = horizontalBloomImage.image;
				attachmentView = horizontalBloomImage.imageView;

				pushConstant.bloomBits = BloomProcessBits::HORIZONTAL;
				horizontal = false;
				extent.width = r_renderTargetContext.imageExtent.width / 2;
				extent.height = r_renderTargetContext.imageExtent.height / 2;

			}
			else
			{
				attachment = verticalBloomImage.image;
				attachmentView = verticalBloomImage.imageView;
				pushConstant.bloomBits = BloomProcessBits::VERTICAL;
				horizontal = true;

				extent.width = r_renderTargetContext.imageExtent.width / 2;
				extent.height = r_renderTargetContext.imageExtent.height / 2;
			}

			if (!enableBloom)
			{
				pushConstant.bloomBits |= BloomProcessBits::DISABLED; 
				extent = r_renderTargetContext.imageExtent;
			}

			transition_image_layout(
				attachment,
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
			colorAttachment.imageView = attachmentView;
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = {
				.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
			};

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
				r_bloomPipelineContext.pipeline
			);

			vkCmdPushConstants(
				commandBuffer,
				r_bloomPipelineContext.layout,
				VK_SHADER_STAGE_VERTEX_BIT |
				VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(BloomPC),
				&pushConstant
			);

			vkCmdBindDescriptorSets(
				commandBuffer,
			    	VK_PIPELINE_BIND_POINT_GRAPHICS,
				r_bloomPipelineContext.layout,
				0, 
				1,
				&post_processDescriptorSets[r_currentFrame],
				0,
				nullptr
			);

			vkCmdDraw(commandBuffer, 6, 1, 0, 0);
			vkCmdEndRendering(commandBuffer);

			transition_image_layout(
				attachment,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_2_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);
		}
	}
}
