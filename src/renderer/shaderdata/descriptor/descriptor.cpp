#include "renderer/shaderdata/descriptor/descriptor.hpp"
#include "core/logs.hpp"
#include "renderer/shaderdata/descriptor/descriptorlayout.hpp"
#include "renderer/shaderdata/descriptor/texture.hpp"
#include "renderer/shaderdata/descriptor/uniformbuffers.hpp"
#include "renderer/vk_types.hpp"
#include <memory.h>

namespace clz::renderer
{
	bool initDescriptors()
	{
		if (!createUniformBuffers())
		{
			clz::log::error("Could not initialize descriptors");
			return false;
		}
		if (!createDescriptorSetLayout())
		{
			clz::log::error("Could not initialize descriptor");
			return false;
		}
		if (!initTextureEngine())
		{
			clz::log::error("Wait, how'd initTextureEngine failed!!???");
			return false;
		}

		// Creating descriptor pool
		std::vector<VkDescriptorPoolSize> poolSize;

		// Transform UBO
		VkDescriptorPoolSize transformUBOPoolSize;
		transformUBOPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		transformUBOPoolSize.descriptorCount = r_FRAMES_IN_FLIGHT;
		poolSize.emplace_back(transformUBOPoolSize);

		// Texture Samplers
		VkDescriptorPoolSize texturePoolSize;
		texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturePoolSize.descriptorCount = r_FRAMES_IN_FLIGHT * r_MAX_TEXTURE_COUNT;
		poolSize.emplace_back(texturePoolSize);

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		poolInfo.poolSizeCount = poolSize.size();
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(r_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(clz::renderer::r_deviceContext.device, &poolInfo, nullptr, &r_descriptorPool) != VK_SUCCESS)
		{
			clz::log::error("Could not create descriptor pool");
			return false;
		}

		// Creating Descriptor sets
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = r_descriptorPool;
		allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
		const std::vector<VkDescriptorSetLayout> layouts(r_FRAMES_IN_FLIGHT, r_descriptorSetLayout);
		allocInfo.pSetLayouts = layouts.data();

		r_descriptorSets.resize(r_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(clz::renderer::r_deviceContext.device, &allocInfo, r_descriptorSets.data()) != VK_SUCCESS)
		{
			clz::log::error("vulkan could not allocate descriptor sets");
			return false;
		}

		for (auto i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo transformBufferInfo = {};
			transformBufferInfo.buffer = r_uniformBuffers;
			transformBufferInfo.offset = r_transformUBOOffsets[i];
			transformBufferInfo.range = sizeof(TransformUBO);

			std::vector<VkWriteDescriptorSet> descriptorWrites;

			VkWriteDescriptorSet transformUBOWrite = {};
			transformUBOWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			transformUBOWrite.dstSet = r_descriptorSets[i];
			transformUBOWrite.dstArrayElement = 0;
			transformUBOWrite.dstBinding = r_TRANSFORM_DESCRIPTOR_BINDING;
			transformUBOWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			transformUBOWrite.descriptorCount = 1;
			transformUBOWrite.pBufferInfo = &transformBufferInfo;
			descriptorWrites.emplace_back(transformUBOWrite);

			vkUpdateDescriptorSets(clz::renderer::r_deviceContext.device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
		}

		clz::log::info("(partially) initialised descriptors");
		return true;
	}

	bool createDescriptors()
	{
		if (r_numRegisteredTextures == 0)
		{
			clz::log::warn("No textures were registered");
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
				write.dstSet = r_descriptorSets[j];
				write.dstBinding = r_TEXTURE_DESCRIPTOR_BIND_POINT;
				write.dstArrayElement = i;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = 1;
				write.pImageInfo = &imageInfo;
				vkUpdateDescriptorSets(clz::renderer::r_deviceContext.device, 1, &write, 0, nullptr);
			}
		}
		clz::log::info("initialized entities texture descriptors");

		return true;
	}

	void updateDescriptors(VkCommandBuffer commandBuffer, const uint32_t currentFrame)
	{
		updateUniformBuffers(currentFrame);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, clz::renderer::r_pipelineContext.layout, 0, 1,
					&r_descriptorSets[r_currentFrame], 0, nullptr);
	}

	void destroyDescriptors()
	{
		vkDestroyDescriptorPool(clz::renderer::r_deviceContext.device, r_descriptorPool, nullptr);
		destroyTextures();
		destroyDescriptorSetLayout();
		destroyUniformBuffers();

		clz::log::info("Destroyed descriptors");
	}

} // namespace clz::renderer
