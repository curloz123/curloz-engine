/**
 * @file descriptor.cpp
 * @author curl0z
 * @brief Creates the descriptor pool, allocates one VkDescriptorSet per
 * frame-in-flight for each of the camera UBO, directional light UBO,
 * combined sampler, and light SSBO set, and writes all buffer bindings.
 */

#include "renderer/pipelinedata/descriptor.hpp"
#include "core/logs.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/pipelinedata/camera.hpp"
#include "renderer/pipelinedata/lights.hpp"
#include "renderer/pipelinedata/texture.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/pipelinedata/post_process.hpp"

namespace clz::renderer
{
	bool createDescriptorPool()
	{
		// --- Pool sized for everything we'll allocate below ---
		std::vector<VkDescriptorPoolSize> poolSize;

		// camera
		VkDescriptorPoolSize cameraUBOPoolSize = {};
		cameraUBOPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraUBOPoolSize.descriptorCount = r_FRAMES_IN_FLIGHT;
		poolSize.emplace_back(cameraUBOPoolSize);

		// Texture samplers
		VkDescriptorPoolSize texturePoolSize = {};
		texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturePoolSize.descriptorCount =
			r_FRAMES_IN_FLIGHT * static_cast<uint32_t>(r_MAX_TEXTURE_COUNT);
		poolSize.emplace_back(texturePoolSize);

		// light's UBO (data)
		VkDescriptorPoolSize lightDataPoolSize = {};
		lightDataPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightDataPoolSize.descriptorCount = r_FRAMES_IN_FLIGHT;
		poolSize.emplace_back(lightDataPoolSize);
		// light's UBO (directional)
		VkDescriptorPoolSize dirPoolSize = {};
		dirPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dirPoolSize.descriptorCount = r_FRAMES_IN_FLIGHT;
		poolSize.push_back(dirPoolSize);
		// light's SSBO (point)
		VkDescriptorPoolSize pointPoolSize = {};
		pointPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pointPoolSize.descriptorCount = r_FRAMES_IN_FLIGHT;
		poolSize.push_back(pointPoolSize);

		// post process pool size
		const auto post_processPoolSizes = getPostProcessDescriptorPoolSizes();
		poolSize.insert(poolSize.end(),
			std::make_move_iterator(post_processPoolSizes.begin()),
			std::make_move_iterator(post_processPoolSizes.end())
		);

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(r_FRAMES_IN_FLIGHT) * 4;

		if (vkCreateDescriptorPool(
			    r_deviceContext.device,
			    &poolInfo,
			    nullptr,
			    &descriptorPool
		    ) != VK_SUCCESS)
		{
			clz::log::error("Could not create descriptor pool");
			return false;
		}
		setHandleName(
			reinterpret_cast<uint64_t>(descriptorPool),
			VK_OBJECT_TYPE_DESCRIPTOR_POOL,
			"main descriptor pool"
		);

		return true;
	}

	bool allocateDescriptorSets()
	{
		/// allocate camera descriptor sets
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
			const std::vector<VkDescriptorSetLayout> layouts(
				r_FRAMES_IN_FLIGHT,
				cameraDescriptorLayout
			);
			allocInfo.pSetLayouts = layouts.data();

			cameraDescriptorSets.resize(r_FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(
				    r_deviceContext.device,
				    &allocInfo,
				    cameraDescriptorSets.data()
			    ) != VK_SUCCESS)
			{
				clz::log::error("vulkan could not allocate camera descriptor sets");
				return false;
			}

			for (uint32_t j = 0; j < r_FRAMES_IN_FLIGHT; ++j)
			{
				setHandleName(
					reinterpret_cast<uint64_t>(cameraDescriptorSets[j]),
					VK_OBJECT_TYPE_DESCRIPTOR_SET,
					("camera descriptor [frame " + std::to_string(j) + "]")
						.c_str()
				);
			}
		}

		// --- Allocate texture descriptor sets, one per frame ---
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
			const std::vector<VkDescriptorSetLayout> layouts(
				r_FRAMES_IN_FLIGHT,
				textureDescriptorLayout
			);
			allocInfo.pSetLayouts = layouts.data();

			textureDescriptorSets.resize(r_FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(
				    r_deviceContext.device,
				    &allocInfo,
				    textureDescriptorSets.data()
			    ) != VK_SUCCESS)
			{
				clz::log::error(
					"vulkan could not allocate sampler descriptor sets"
				);
				return false;
			}

			for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
			{
				setHandleName(
					reinterpret_cast<uint64_t>(textureDescriptorSets[i]),
					VK_OBJECT_TYPE_DESCRIPTOR_SET,
					("sampler descriptor set [frame " + std::to_string(i) + "]")
						.c_str()
				);
			}
		}

		// --- Allocate light descriptor set, one per frame ---
		// --- two UBO and two SSBO's
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
			const std::vector<VkDescriptorSetLayout> layouts(
				r_FRAMES_IN_FLIGHT,
				lightDescriptorLayout
			);
			allocInfo.pSetLayouts = layouts.data();

			lightDescriptorSets.resize(r_FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(
				    r_deviceContext.device,
				    &allocInfo,
				    lightDescriptorSets.data()
			    ) != VK_SUCCESS)
			{
				clz::log::error("vulkan could not allocate light descriptor sets");
				return false;
			}

			for (uint32_t j = 0; j < r_FRAMES_IN_FLIGHT; ++j)
			{
				setHandleName(
					reinterpret_cast<uint64_t>(cameraDescriptorSets[j]),
					VK_OBJECT_TYPE_DESCRIPTOR_SET,
					("light descriptor [frame " + std::to_string(j) + "]")
						.c_str()
				);
			}
		}

		// allocate post process descriptor sets
		allocatePostProcessDescriptorSets(descriptorPool);


		// --- Write camera descriptor sets ---
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo cameraInfo = {};
			cameraInfo.buffer = cameraUBOMemory.buffer;
			cameraInfo.offset = cameraUBO.offsets[i];
			cameraInfo.range = cameraUBO.uboSize;

			VkWriteDescriptorSet cameraWrite{};
			cameraWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			cameraWrite.dstSet = cameraDescriptorSets[i];
			cameraWrite.dstBinding = cameraUBO.uboBindingPoint;
			cameraWrite.dstArrayElement = 0;
			cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			cameraWrite.descriptorCount = 1;
			cameraWrite.pBufferInfo = &cameraInfo;

			vkUpdateDescriptorSets(r_deviceContext.device, 1, &cameraWrite, 0, nullptr);
		}

		// texture set update is done after entities have been created

		// --- Write light descriptor sets ---
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			std::vector<VkWriteDescriptorSet> lightWrites;
			lightWrites.reserve(3);

			// light data
			VkDescriptorBufferInfo dataInfo = {};
			dataInfo.buffer = lightDataUBOMemory.buffer;
			dataInfo.offset = lightDataUBO.offsets[i];
			dataInfo.range = lightDataUBO.uboSize;

			VkWriteDescriptorSet lightDataWrite{};
			lightDataWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			lightDataWrite.dstSet = lightDescriptorSets[i];
			lightDataWrite.dstBinding = lightDataUBO.uboBindingPoint;
			lightDataWrite.dstArrayElement = 0;
			lightDataWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			lightDataWrite.descriptorCount = 1;
			lightDataWrite.pBufferInfo = &dataInfo;

			lightWrites.push_back(lightDataWrite);
			/// directional light
			VkDescriptorBufferInfo lightInfo = {};
			lightInfo.buffer = dirUBOMemory.buffer;
			lightInfo.offset = dirUBO.offsets[i];
			lightInfo.range = dirUBO.uboSize;

			VkWriteDescriptorSet lightWrite = {};
			lightWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			lightWrite.dstSet = lightDescriptorSets[i];
			lightWrite.dstBinding = dirUBO.uboBindingPoint;
			lightWrite.dstArrayElement = 0;
			lightWrite.descriptorCount = 1;
			lightWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			lightWrite.pBufferInfo = &lightInfo;

			lightWrites.push_back(lightWrite);

			/// point light
			VkDescriptorBufferInfo pointLightInfo = {};
			pointLightInfo.buffer = pointSSBOMemory.buffer;
			pointLightInfo.offset = pointSSBO.offsets[i];
			pointLightInfo.range = pointSSBO.ssboSize;

			VkWriteDescriptorSet pointLightWrite = {};
			pointLightWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			pointLightWrite.dstSet = lightDescriptorSets[i];
			pointLightWrite.dstBinding = pointSSBO.ssboBindingPoint;
			pointLightWrite.dstArrayElement = 0;
			pointLightWrite.descriptorCount = 1;
			pointLightWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			pointLightWrite.pBufferInfo = &pointLightInfo;

			lightWrites.push_back(pointLightWrite);


			vkUpdateDescriptorSets(
				r_deviceContext.device,
				static_cast<uint32_t>(lightWrites.size()),
				lightWrites.data(),
				0,
				nullptr
			);

		}

		// post process descriptor are handled by post_process system

		clz::log::info("Created and wrote most descriptor sets");
		return true;
	}

	void destroyDescriptorPool()
	{
		vkDestroyDescriptorPool(r_deviceContext.device, descriptorPool, nullptr);
		lightDescriptorSets.clear();
		textureDescriptorSets.clear();
		cameraDescriptorSets.clear();
	}
} // namespace clz::renderer
