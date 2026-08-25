/**
 * @file post_process.cpp
 * @author curl0z
 * @brief post processes pipeline data implementation file
 */
#include "renderer/pipelinedata/post_process.hpp"
#include "core/logs.hpp"
#include "renderer/postprocess/bloom.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/postprocess/bloom_sample.hpp"
#include "renderer/postprocess/tonemap.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/descriptor.hpp"
#include <cstdint>
#include <array>
#include <vulkan/vulkan_core.h>

#ifdef CLZ_ENABLE_EDITOR
#include "include/sceneview.hpp"
#endif

namespace clz::renderer
{
	/// @copydoc createPostProcessDescriptor
	bool createPostProcessDescriptor()
	{
		constexpr uint8_t count = 6;

		constexpr std::array<uint32_t, count> bindPoints = {
			RENDER_TARGET_IMAGE_BIND_POINT,
			BLOOM_SAMPLE_IMAGE_BIND_POINT,
			BLOOM_MIP_IMAGES_BIND_POINT,
			BLOOMED_IMAGE_BIND_POINT,
			TONEMAP_IMAGE_BIND_POINT,
			POST_TONEMAP_IMAGE_BIND_POINT
		};
		constexpr std::array<VkDescriptorType, count> descriptorTypes = {
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		};
		constexpr std::array<uint32_t, count> descriptorCounts = {1, 1, post_process::NUM_BLOOM_MIPS, 1, 1, 1};
		constexpr std::array<VkShaderStageFlags, count> shaderStages = {
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
		};
		if (!createDescriptorLayout(
			post_processDescriptorLayout,
			bindPoints,
			descriptorTypes,
			descriptorCounts,
			shaderStages,
			"post-process descriptor layout",
			true))
		{
			clz::log::error("unable to create post-process descriptor layout");
			return false;
		}

		return true;
	}

	/// @copydoc getPostProcessDescriptorPoolSizes
	std::vector<VkDescriptorPoolSize> getPostProcessDescriptorPoolSizes()
	{
		std::vector<VkDescriptorPoolSize> poolSizes;

		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		/**
		 * One for render target
		 * One for Bloom sample
		 * 5 for bloom mips
		 * One for Bloomed
		 * One for tonemap
		 * One for post_tonemap
		 */
		poolSize.descriptorCount = r_FRAMES_IN_FLIGHT * 10;
		poolSizes.push_back(poolSize);

		return poolSizes;
	}

	/// @copydoc allocatePostProcessDescriptorSets
	bool allocatePostProcessDescriptorSets(
		const VkDescriptorPool& descriptorPool)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
		const std::vector<VkDescriptorSetLayout> layouts(
			r_FRAMES_IN_FLIGHT,
			post_processDescriptorLayout
		);
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(
			    r_deviceContext.device,
			    &allocInfo,
			    post_processDescriptorSets.data()
		    ) != VK_SUCCESS)
		{
			clz::log::error("vulkan could not allocate post-process descriptor sets");
			return false;
		}

		for (uint32_t j = 0; j < r_FRAMES_IN_FLIGHT; ++j)
		{
			setHandleName(
				reinterpret_cast<uint64_t>(post_processDescriptorSets[j]),
				VK_OBJECT_TYPE_DESCRIPTOR_SET,
				("post-process descriptor [frame " + std::to_string(j) + "]").c_str()
			);
		}

		return true;
	}

	/// @copydoc updatePostProcessDescriptorSets
	void updatePostProcessDescriptorSets()
	{
		VkDescriptorImageInfo renderTargetInfo = {};
		renderTargetInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		renderTargetInfo.imageView = r_renderTargetContext.imageView;
		renderTargetInfo.sampler = r_renderTargetContext.imageSampler;

		VkDescriptorImageInfo bloomSampleInfo = {};
		bloomSampleInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		bloomSampleInfo.imageView = post_process::bloomSampleImageView;
		bloomSampleInfo.sampler = post_process::bloomSampleSampler;

		std::array<VkDescriptorImageInfo, post_process::NUM_BLOOM_MIPS> bloomMipInfos;
		for (int i = 0; i < post_process::NUM_BLOOM_MIPS; ++i)
		{
			bloomMipInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			bloomMipInfos[i].imageView = post_process::bloomMips[i].imageView;
			bloomMipInfos[i].sampler = post_process::bloomSampler;
		}

		VkDescriptorImageInfo bloomBlendInfo = {};
		bloomBlendInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		bloomBlendInfo.imageView = post_process::bloomedImage.imageView;
		bloomBlendInfo.sampler = post_process::bloomSampler;

		VkDescriptorImageInfo tonemapInfo = {};
		tonemapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		tonemapInfo.imageView = post_process::tonemapImageView;
		tonemapInfo.sampler = post_process::tonemapSampler;

		VkDescriptorImageInfo postTonemapInfo = {};
		postTonemapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		postTonemapInfo.imageView = post_process::postTonemapImageView;
		postTonemapInfo.sampler = post_process::postTonemapSampler;

		for (uint32_t j = 0; j < r_FRAMES_IN_FLIGHT; ++j)
		{
			VkWriteDescriptorSet renderTargetWrite{};
			renderTargetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			renderTargetWrite.dstSet = post_processDescriptorSets[j];
			renderTargetWrite.dstBinding = RENDER_TARGET_IMAGE_BIND_POINT;
			renderTargetWrite.dstArrayElement = 0;
			renderTargetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			renderTargetWrite.descriptorCount = 1;
			renderTargetWrite.pImageInfo = &renderTargetInfo;

			VkWriteDescriptorSet bloomSampleWrite{};
			bloomSampleWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			bloomSampleWrite.dstSet = post_processDescriptorSets[j];
			bloomSampleWrite.dstBinding = BLOOM_SAMPLE_IMAGE_BIND_POINT;
			bloomSampleWrite.dstArrayElement = 0;
			bloomSampleWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bloomSampleWrite.descriptorCount = 1;
			bloomSampleWrite.pImageInfo = &bloomSampleInfo;

			VkWriteDescriptorSet bloomMipWrite{};
			bloomMipWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			bloomMipWrite.dstSet = post_processDescriptorSets[j];
			bloomMipWrite.dstBinding = BLOOM_MIP_IMAGES_BIND_POINT;
			bloomMipWrite.dstArrayElement = 0;
			bloomMipWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bloomMipWrite.descriptorCount = post_process::NUM_BLOOM_MIPS;
			bloomMipWrite.pImageInfo = bloomMipInfos.data();

			VkWriteDescriptorSet bloomWrite = {};
			bloomWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			bloomWrite.dstSet = post_processDescriptorSets[j];
			bloomWrite.dstArrayElement = 0;
			bloomWrite.dstBinding = BLOOMED_IMAGE_BIND_POINT;
			bloomWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bloomWrite.descriptorCount = 1;
			bloomWrite.pImageInfo = &bloomBlendInfo;

			VkWriteDescriptorSet tonemapWrite{};
			tonemapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			tonemapWrite.dstSet = post_processDescriptorSets[j];
			tonemapWrite.dstBinding = TONEMAP_IMAGE_BIND_POINT;
			tonemapWrite.dstArrayElement = 0;
			tonemapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			tonemapWrite.descriptorCount = 1;
			tonemapWrite.pImageInfo = &tonemapInfo;

			VkWriteDescriptorSet postTonemapWrite{};
			postTonemapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			postTonemapWrite.dstSet = post_processDescriptorSets[j];
			postTonemapWrite.dstBinding = POST_TONEMAP_IMAGE_BIND_POINT;
			postTonemapWrite.dstArrayElement = 0;
			postTonemapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			postTonemapWrite.descriptorCount = 1;
			postTonemapWrite.pImageInfo = &postTonemapInfo;

			const std::array<VkWriteDescriptorSet, 6> writes = {
				renderTargetWrite,
				bloomSampleWrite,
				bloomMipWrite,
				bloomWrite,
				tonemapWrite, 
				postTonemapWrite
			};

			vkUpdateDescriptorSets(
				r_deviceContext.device,
				static_cast<uint32_t>(writes.size()),
				writes.data(),
				0,
				nullptr
			);
		}
	}

	/// @copydoc destroyPostProcessDescriptor
	void destroyPostProcessDescriptor()
	{
		vkDestroyDescriptorSetLayout(
			r_deviceContext.device,
			post_processDescriptorLayout,
			nullptr);
	}

}
