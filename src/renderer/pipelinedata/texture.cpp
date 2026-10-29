/**
 * @file texture.cpp
 * @author curl0z
 * @brief Implementation of the bindless texture/sampler descriptor interface.
 */

#include "renderer/pipelinedata/texture.hpp"
#include "core/logs.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/model/model.hpp"
#include "renderer/model/model_material.hpp"
#include "renderer/utility/descriptor.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include <vulkan/vulkan_core.h>

namespace clz::renderer
{
	bool TextureDescriptorInterface::initialize()
	{
		const std::vector<uint32_t> bindPoints = {
			TEXTURE_BIND_POINT,
			SAMPLER_BIND_POINT,
			MATERIAL_BIND_POINT,
		};
		const std::vector<VkDescriptorType> descriptorTypes = {
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			VK_DESCRIPTOR_TYPE_SAMPLER,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		};
		const std::vector<uint32_t> descriptorCounts = {
			MAX_TEXTURE_COUNT,
			MAX_SAMPLER_COUNT,
			MAX_MATERIAL_COUNT
		};
		const std::vector<VkShaderStageFlags> shaderStages = {
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT
		};
		if (!createDescriptorLayout(
			    m_descriptorLayout,
			    bindPoints,
			    descriptorTypes,
			    descriptorCounts,
			    shaderStages,
			    "texture descriptor layout",
			    true
		    ))
		{
			clz::log::error("unable to create texture descriptor layout");
			return false;
		}

		return true;
	}

	void TextureDescriptorInterface::destroy()
	{
		destroyDescriptorSetLayout(m_descriptorLayout);
	}

	std::vector<VkDescriptorPoolSize> TextureDescriptorInterface::getRequiredPoolSizes()
	{
		std::vector<VkDescriptorPoolSize> poolSizes(3);

		poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		poolSizes[0].descriptorCount = r_FRAMES_IN_FLIGHT * MAX_TEXTURE_COUNT;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		poolSizes[1].descriptorCount = r_FRAMES_IN_FLIGHT * MAX_SAMPLER_COUNT;

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = r_FRAMES_IN_FLIGHT * MAX_MATERIAL_COUNT;

		return poolSizes;
	}

	bool TextureDescriptorInterface::allocateDescriptorSets(VkDescriptorPool descriptorPool)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
		const std::vector<VkDescriptorSetLayout> layouts(
			r_FRAMES_IN_FLIGHT,
			m_descriptorLayout
		);
		allocInfo.pSetLayouts = layouts.data();

		m_descriptorSets.resize(r_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(
			    r_deviceContext.device,
			    &allocInfo,
			    m_descriptorSets.data()
		    ) != VK_SUCCESS)
		{
			clz::log::error("vulkan could not allocate texture descriptor sets");
			return false;
		}

		for (size_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			setHandleName(
				reinterpret_cast<uint64_t>(m_descriptorSets[i]),
				VK_OBJECT_TYPE_DESCRIPTOR_SET,
				("texture descriptor [frame " + std::to_string(i) + "]").c_str()
			);
		}

		return true;
	}

	void TextureDescriptorInterface::updateDescriptorSets()
	{
		const uint32_t numTextures = TextureManager.getTextureArrayLimit();
		const uint32_t numSamplers = TextureManager.getSamplerArrayLimit();

		const auto& materialManager = ModelManager.getMaterialManager();
		const uint32_t numMaterials = materialManager.getMaterialArrayLimit();

		if (numTextures == 0)
		{
			clz::log::warn("No textures registered, skipping texture descriptor update");
			return;
		}

		std::vector<VkDescriptorImageInfo> imageInfos(numTextures);
		for (size_t i = 0; i < numTextures; ++i)
		{
			const auto& imageView = TextureManager.getImageView(TextureId(static_cast<uint32_t>(i)));
			if (imageView == VK_NULL_HANDLE)
				continue;

			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfos[i].imageView = imageView;
			imageInfos[i].sampler = VK_NULL_HANDLE;
		}

		std::vector<VkDescriptorImageInfo> samplerInfos(numSamplers);
		for (size_t i = 0; i < numSamplers; ++i)
		{
			const auto& sampler = TextureManager.getSampler(SamplerId(static_cast<uint32_t>(i)));
			if (sampler == VK_NULL_HANDLE)
				continue;

			samplerInfos[i].sampler = sampler;
			samplerInfos[i].imageView = VK_NULL_HANDLE;
			samplerInfos[i].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		const SSBO& materialSSBO = materialManager.getStorageBuffer();

		for (uint32_t j = 0; j < r_FRAMES_IN_FLIGHT; ++j)
		{
			std::vector<VkWriteDescriptorSet> writes;
			writes.reserve(3);

			VkWriteDescriptorSet imageWrite{};
			imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			imageWrite.dstSet = m_descriptorSets[j];
			imageWrite.dstBinding = TEXTURE_BIND_POINT;
			imageWrite.dstArrayElement = 0;
			imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			imageWrite.descriptorCount = numTextures;
			imageWrite.pImageInfo = imageInfos.data();
			writes.push_back(imageWrite);

			if (numSamplers > 0)
			{
				VkWriteDescriptorSet samplerWrite{};
				samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				samplerWrite.dstSet = m_descriptorSets[j];
				samplerWrite.dstBinding = SAMPLER_BIND_POINT;
				samplerWrite.dstArrayElement = 0;
				samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				samplerWrite.descriptorCount = numSamplers;
				samplerWrite.pImageInfo = samplerInfos.data();
				writes.push_back(samplerWrite);
			}

			VkDescriptorBufferInfo materialInfo{};
			if (numMaterials > 0)
			{
				materialInfo.buffer = materialSSBO.buffer;
				materialInfo.offset = materialSSBO.offsets[j];
				materialInfo.range = sizeof(ModelMaterial) * numMaterials;

				VkWriteDescriptorSet materialWrite{};
				materialWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				materialWrite.dstSet = m_descriptorSets[j];
				materialWrite.dstBinding = MATERIAL_BIND_POINT;
				materialWrite.dstArrayElement = 0;
				materialWrite.descriptorCount = 1;
				materialWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				materialWrite.pBufferInfo = &materialInfo;
				writes.push_back(materialWrite);
			}

			vkUpdateDescriptorSets(
				r_deviceContext.device,
				static_cast<uint32_t>(writes.size()),
				writes.data(),
				0,
				nullptr
			);
		}

		clz::log::info("updated texture descriptor sets");
	}
} // namespace clz::renderer
