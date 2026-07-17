/**
 * @file descriptor.cpp
 * @author curl0z
 * @brief Creates the descriptor pool, allocates one VkDescriptorSet per
 * frame-in-flight for each of the camera UBO and the combined sampler
 * (each from its own layout, per ubo.hpp / sampler.hpp), and writes the
 * camera UBO's buffer bindings.
 */

#include "renderer/pipelinedata/descriptor.hpp"
#include "core/logs.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/pipelinedata/sampler.hpp"
#include "renderer/pipelinedata/ubo.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	bool createDescriptors()
	{
		// --- Pool sized for everything we'll allocate below ---
		std::vector<VkDescriptorPoolSize> poolSize;

		// UBOs (camera, and any future ones counted via NUM_UNIFORM_BUFFERS)
		VkDescriptorPoolSize UBOPoolSize = {};
		UBOPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		UBOPoolSize.descriptorCount = r_FRAMES_IN_FLIGHT * NUM_UNIFORM_BUFFERS;
		poolSize.emplace_back(UBOPoolSize);

		// Texture Samplers
		VkDescriptorPoolSize texturePoolSize = {};
		texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturePoolSize.descriptorCount = r_FRAMES_IN_FLIGHT * r_MAX_TEXTURE_COUNT;
		poolSize.emplace_back(texturePoolSize);

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		poolInfo.pPoolSizes = poolSize.data();
		// Two sets allocated PER frame-in-flight (one camera, one sampler) -> 2x maxSets
		poolInfo.maxSets = static_cast<uint32_t>(r_FRAMES_IN_FLIGHT) * 2;

		if (vkCreateDescriptorPool(r_deviceContext.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			clz::log::error("Could not create descriptor pool");
			return false;
		}
		setHandleName(reinterpret_cast<uint64_t>(descriptorPool), VK_OBJECT_TYPE_DESCRIPTOR_POOL, "main descriptor pool");

		// --- Allocate camera UBO descriptor sets, one per frame-in-flight ---
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
			const std::vector<VkDescriptorSetLayout> layouts(r_FRAMES_IN_FLIGHT, cameraUBOLayout);
			allocInfo.pSetLayouts = layouts.data();

			cameraDescriptorSets.resize(r_FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(r_deviceContext.device, &allocInfo, cameraDescriptorSets.data()) != VK_SUCCESS)
			{
				clz::log::error("vulkan could not allocate camera descriptor sets");
				return false;
			}

			for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
			{
				setHandleName(reinterpret_cast<uint64_t>(cameraDescriptorSets[i]), VK_OBJECT_TYPE_DESCRIPTOR_SET,
					      ("camera descriptor set [frame " + std::to_string(i) + "]").c_str());
			}
		}

		// --- Allocate sampler descriptor sets, one per frame-in-flight ---
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
			const std::vector<VkDescriptorSetLayout> layouts(r_FRAMES_IN_FLIGHT, combinedSamplerLayout);
			allocInfo.pSetLayouts = layouts.data();

			samplerDescriptorSets.resize(r_FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(r_deviceContext.device, &allocInfo, samplerDescriptorSets.data()) != VK_SUCCESS)
			{
				clz::log::error("vulkan could not allocate sampler descriptor sets");
				return false;
			}

			for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
			{
				setHandleName(reinterpret_cast<uint64_t>(samplerDescriptorSets[i]), VK_OBJECT_TYPE_DESCRIPTOR_SET,
					      ("sampler descriptor set [frame " + std::to_string(i) + "]").c_str());
			}
		}

		// --- Write the camera UBO's buffer binding into each of its descriptor sets ---
		// (Sampler bindings are written later via updateSamplersDataForDescriptorSets,
		//  once scene textures actually exist to point at.)
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uboMemory.buffer;
			bufferInfo.offset = cameraUBO.offsets[i];
			bufferInfo.range = cameraUBO.uboSize;

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = cameraDescriptorSets[i];
			write.dstBinding = cameraUBO.uboBindingPoint;
			write.dstArrayElement = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(r_deviceContext.device, 1, &write, 0, nullptr);
		}

		clz::log::info("Created and wrote camera descriptor sets; sampler sets allocated, awaiting texture data");
		return true;
	}

	void destroyDescriptors()
	{
		// Destroying the pool implicitly frees every set allocated from it.
		vkDestroyDescriptorPool(r_deviceContext.device, descriptorPool, nullptr);
		cameraDescriptorSets.clear();
		samplerDescriptorSets.clear();
	}
} // namespace clz::renderer