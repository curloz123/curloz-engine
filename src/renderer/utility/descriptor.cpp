#include "renderer/entitydata/texture.hpp"
#include "core/logs.hpp"
#include "renderer/utility/descriptor.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/pipelineinput/globalinput.hpp"

namespace clz::renderer
{
	bool createDescriptor(PipelineContext& pipelineContext, const bool enableUBO,
		const std::vector<UBO*>& pUniformBuffers, UBOMemory& uboMemory,
		const bool enableTextureBuffer)
	{
		if (!createUniformBuffers(pUniformBuffers, uboMemory))
		{
			clz::log::error("Could not create uniform buffers!");
			return false;
		}

		// Creating descriptor pool
		std::vector<VkDescriptorPoolSize> poolSize;

		// Texture Samplers
		VkDescriptorPoolSize texturePoolSize = {};
		if (enableTextureBuffer)
		{
			texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			texturePoolSize.descriptorCount = r_FRAMES_IN_FLIGHT * r_MAX_TEXTURE_COUNT;
			poolSize.emplace_back(texturePoolSize);
		}

		// UBO
		VkDescriptorPoolSize UBOPoolSize = {};
		if (enableUBO)
		{
			UBOPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			UBOPoolSize.descriptorCount = r_FRAMES_IN_FLIGHT * pUniformBuffers.size();
			poolSize.emplace_back(UBOPoolSize);
		}

		if (poolSize.empty())
		{
			clz::log::warn("No texture, no UBO's, "
					"Why are you creating a descriptor??");
			return true;
		}

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		if (enableTextureBuffer)
		{
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		}
		poolInfo.poolSizeCount = poolSize.size();
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(r_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(r_deviceContext.device, &poolInfo,
			nullptr, &pipelineContext.descriptorPool) != VK_SUCCESS)
		{
			clz::log::error("Could not create descriptor pool");
			return false;
		}

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pipelineContext.descriptorPool;
		allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
		const std::vector<VkDescriptorSetLayout> layouts(r_FRAMES_IN_FLIGHT, pipelineContext.descriptorSetLayout);
		allocInfo.pSetLayouts = layouts.data();

		pipelineContext.descriptorSets.resize(r_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(r_deviceContext.device, &allocInfo,
			pipelineContext.descriptorSets.data()) != VK_SUCCESS)
		{
			clz::log::error("vulkan could not allocate descriptor sets");
			return false;
		}

		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			std::vector<VkWriteDescriptorSet> descriptorWrites;

			for (uint32_t j = 0; j < pUniformBuffers.size(); ++j)
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = uboMemory.buffer;
				bufferInfo.offset = pUniformBuffers[j]->offsets[i];
				bufferInfo.range = pUniformBuffers[j]->uboSize;

				clz::log::debug("binding point: " + std::to_string(pUniformBuffers[j]->uboBindingPoint));
				VkWriteDescriptorSet cameraUBOWrite = {};
				cameraUBOWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				cameraUBOWrite.dstSet = pipelineContext.descriptorSets[i];
				cameraUBOWrite.dstArrayElement = 0;
				cameraUBOWrite.dstBinding = pUniformBuffers[j]->uboBindingPoint;
				cameraUBOWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				cameraUBOWrite.descriptorCount = 1;
				cameraUBOWrite.pBufferInfo = &bufferInfo;
				descriptorWrites.emplace_back(cameraUBOWrite);
			}
			vkUpdateDescriptorSets(r_deviceContext.device, static_cast<uint32_t>(descriptorWrites.size()),
					descriptorWrites.data(), 0, nullptr);
		}

		clz::log::info("Partially created descriptors");

		return true;
	}

	void destroyDescriptor(PipelineContext& pipelineContext, UBOMemory& uboMemory)
	{
		vkDestroyDescriptorPool(r_deviceContext.device, pipelineContext.descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(r_deviceContext.device, pipelineContext.descriptorSetLayout, nullptr);
		destroyUniformBuffer(uboMemory);
		clz::log::info("Destroyed descriptors");
	}


	void updateTextureData(PipelineContext& pipelineContext)
	{
		if (r_numRegisteredTextures == 0)
		{
			clz::log::warn("No textures registered, "
					"not updating entity data in descriptor");
			return;
		}
		for (uint32_t i = 0; i < r_numRegisteredTextures; ++i)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = r_textures.imageView[i];
			imageInfo.sampler = r_sampler;

			for (auto j = 0; j < r_FRAMES_IN_FLIGHT; ++j)
			{
				VkWriteDescriptorSet write{};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = pipelineContext.descriptorSets[j];
				write.dstBinding = TEXTURE_DESCRIPTOR_BIND_POINT;
				write.dstArrayElement = i;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = 1;
				write.pImageInfo = &imageInfo;
				vkUpdateDescriptorSets(clz::renderer::r_deviceContext.device, 1, &write, 0, nullptr);
			}
		}
		clz::log::info("initialized entities texture descriptors");
	}
}