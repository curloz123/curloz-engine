#include "renderer/pipelinedata/post_process.hpp"

#include "core/enginestate.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/postprocess/pre_tonemap.hpp"
#include "renderer/postprocess/tonemap.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/utility/namer.hpp"

#include <cstdint>
#include <array>

#ifdef CLZ_ENABLE_EDITOR
#include "include/sceneview.hpp"
#endif

// bloom is not implemented yet

namespace clz::renderer
{
	bool createPostProcessDescriptor()
	{
		constexpr uint8_t count = 3;

		constexpr std::array<uint32_t, count> bindPoints = {
			PRE_TONEMAP_IMAGE_BIND_POINT,
			TONEMAP_IMAGE_BIND_POINT,
			POST_TONEMAP_IMAGE_BIND_POINT
		};
		constexpr std::array<VkDescriptorType, count> descriptorTypes = {
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		constexpr std::array<uint32_t, count> descriptorCounts = {1, 1, 1};
		constexpr std::array<VkShaderStageFlags, count> shaderStages = {
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT
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

#ifndef CLZ_ENABLE_EDITOR
		updatePostProcessDescriptorSets();
#endif

		return true;
	}

	std::vector<VkDescriptorPoolSize> getPostProcessDescriptorPoolSizes()
	{
		std::vector<VkDescriptorPoolSize> poolSizes;

		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = r_FRAMES_IN_FLIGHT * 3;
		poolSizes.push_back(poolSize);

		return poolSizes;
	}

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

	void updatePostProcessDescriptorSets()
	{
		VkDescriptorImageInfo preTonemapInfo = {};
		preTonemapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		preTonemapInfo.imageView = r_renderTargetContext.imageView;
		preTonemapInfo.sampler = r_renderTargetContext.imageSampler;

		VkDescriptorImageInfo tonemapInfo = {};
		tonemapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		tonemapInfo.imageView = post_process::preTonemapImageView;
		tonemapInfo.sampler = post_process::preTonemapSampler;

		VkDescriptorImageInfo postTonemapInfo = {};
		postTonemapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		postTonemapInfo.imageView = post_process::tonemapImageView;
		postTonemapInfo.sampler = post_process::tonemapSampler;

		for (uint32_t j = 0; j < r_FRAMES_IN_FLIGHT; ++j)
		{
			VkWriteDescriptorSet preTonemapWrite{};
			preTonemapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			preTonemapWrite.dstSet = post_processDescriptorSets[j];
			preTonemapWrite.dstBinding = PRE_TONEMAP_IMAGE_BIND_POINT;
			preTonemapWrite.dstArrayElement = 0;
			preTonemapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			preTonemapWrite.descriptorCount = 1;
			preTonemapWrite.pImageInfo = &preTonemapInfo;

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

			const std::array<VkWriteDescriptorSet, 3> writes = {
				preTonemapWrite, tonemapWrite, postTonemapWrite
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

	void destroyPostProcessDescriptor()
	{
		vkDestroyDescriptorSetLayout(
			r_deviceContext.device,
			post_processDescriptorLayout,
			nullptr);
	}

}