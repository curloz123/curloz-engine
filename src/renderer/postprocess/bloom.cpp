/**
 * @file bloom.cpp
 * @author curl0z
 * @brief Bloom post process implementation file
 */
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
#include "renderer/config.hpp"

namespace clz::renderer::post_process
{
	/// @copydoc createBloomProcess
	bool createBloomProcess()
	{
		const uint32_t width = std::max(r_renderTargetContext.imageExtent.width, 1u);
		const uint32_t height = std::max(r_renderTargetContext.imageExtent.height, 1u);
		auto createBloomImage = [](auto& rBloomImage, const VkExtent2D extent, const uint32_t index)
		{
			rBloomImage.extent.width = extent.width;
			rBloomImage.extent.height = extent.height;
			if (!createImage(
				rBloomImage.image,
				"bloom image: " + std::to_string(index),
				extent.width,
				extent.height,
				BLOOM_IMAGE_FORMAT,
				VK_IMAGE_TILING_OPTIMAL,
			    	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
			    		VK_IMAGE_USAGE_SAMPLED_BIT,
				0
			))
			{
				clz::log::error("failed to create bloom image: " + std::to_string(index));
				return false;
			}

			VkMemoryRequirements memReq;
			vkGetImageMemoryRequirements(
				renderer::r_deviceContext.device,
				rBloomImage.image,
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
				    &rBloomImage.memory
			    ) != VK_SUCCESS)
			{
				clz::log::error("failed to allocate bloom memory");
				return false;
			}
			vkBindImageMemory(
				r_deviceContext.device,
				rBloomImage.image,
				rBloomImage.memory,
				0
			);
			setHandleName(
				reinterpret_cast<uint64_t>(rBloomImage.memory),
				VK_OBJECT_TYPE_DEVICE_MEMORY,
				"bloom memory"
			);
			
			if (!createImageView(
					rBloomImage.imageView,
					"bloom image view",
					rBloomImage.image,
					BLOOM_IMAGE_FORMAT,
					VK_IMAGE_ASPECT_COLOR_BIT)
			)
			{
				clz::log::error("failed to create bloom image view: " + std::to_string(index));
				return false;
			}
			
			return true;
		};
		for (int i = 0; i < NUM_BLOOM_MIPS; ++i)
		{
			const auto w = std::max(width / ((i + 1)*2), 1u);
			const auto h = std::max(height / ((i + 1)*2), 1u);
			VkExtent2D extent{
				.width = w,
				.height = h
			};
			
			if (!createBloomImage(
				bloomMips[i],
				extent,
				i)
			)
			{
				clz::log::error("Failed to create bloom image: " + std::to_string(i));
				return false;
			}
		}
		if (!createBloomImage(bloomedImage, r_renderTargetContext.imageExtent, 0))
		{
			clz::log::error("Failed to create bloomed image: ");
			return false;
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

		// retrieve back data from config
		setBloomStrength(bloomStrengthFromConfig());
		setFilterRadius(bloomFilterRadiusFromConfig());

		clz::log::info("Created bloom post-process");
		return true;
	}

	/// @copydoc destroyBloomProcess
	void destroyBloomProcess()
	{
		vkDestroySampler(r_deviceContext.device, bloomSampler, nullptr);
		for (int i = 0; i < NUM_BLOOM_MIPS; ++i)
		{
			vkDestroyImageView(r_deviceContext.device, bloomMips[i].imageView, nullptr);
			vkDestroyImage(r_deviceContext.device, bloomMips[i].image, nullptr);
			vkFreeMemory(r_deviceContext.device, bloomMips[i].memory, nullptr);
		}
	}

	/// @copydoc applyBloomProcess
	void applyBloomProcess(VkCommandBuffer commandBuffer)
	{
		auto performBloom = 
			[commandBuffer](
				VkImage& rImage, 
				VkImageView& rImageView, 
				const VkExtent2D extent, 
				BloomPC* pPushConstant)
		{
			transition_image_layout(
				rImage,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ACCESS_NONE,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
				VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);

			VkRenderingAttachmentInfoKHR colorAttachment = {};
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			colorAttachment.pNext = nullptr;
			colorAttachment.imageView = rImageView;
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
				pPushConstant
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
				rImage,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_2_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);

		};

		if (!Bloom)
		{
			BloomPC pushConstant;
			pushConstant.bloomBits = BloomProcessBits::DISABLE;
			performBloom(
				bloomedImage.image,
				bloomedImage.imageView,
				bloomedImage.extent,
				&pushConstant
			);

			return;
		}

		// down sample proces
		for (int i = -1; i < NUM_BLOOM_MIPS - 1; ++i)
		{
			BloomPC pushConstant;
			pushConstant.bloomBits = BloomProcessBits::DOWNSAMPLE;
			pushConstant.downIndex = i;
			pushConstant.bloomStrength = bloomStrength;
			pushConstant.filterRadius = filterRadius;

			performBloom(
				bloomMips[i+1].image,
				bloomMips[i+1].imageView,
				bloomMips[i+1].extent,
				&pushConstant
			);
		}

		// upsample
		for (int i = NUM_BLOOM_MIPS - 2; i >= -1; --i)
		{
			BloomPC pushConstant;
			pushConstant.bloomBits = BloomProcessBits::UPSAMPLE;
			pushConstant.upIndex = i;
			pushConstant.bloomStrength = bloomStrength;
			pushConstant.filterRadius = filterRadius;

			if (i == -1)
			{
				performBloom(
					bloomedImage.image,
					bloomedImage.imageView,
					bloomedImage.extent,
					&pushConstant
				);
			}
			else
			{
				performBloom(
					bloomMips[i].image,
					bloomMips[i].imageView,
					bloomMips[i].extent,
					&pushConstant
				);
			}
		}
	}
}
