/*
#include "../../../include/offscreen/backend/descriptor.hpp"
#include "../../../../include/renderer/entitydata/texture.hpp"
#include "renderer/utility/descriptor.hpp"
#include "renderer/pipelineinput/globalinput.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include <vulkan/vulkan.h>

namespace clz::editor::backend
{
	bool initializeDescriptors()
	{
		/// #000 Uniform Buffers
		std::vector<renderer::UBO*> UBOs = {&editorCameraUBO};
		renderer::createUniformBuffers(UBOs, uboMemory);

		/// #001 LAYOUT
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		// Descriptor type - 1
		VkDescriptorSetLayoutBinding cameraUBOLayoutBinding = {};
		cameraUBOLayoutBinding.binding = editorCameraUBO.uboBindingPoint;
		cameraUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraUBOLayoutBinding.descriptorCount = 1;
		cameraUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings.push_back(cameraUBOLayoutBinding);

		// Descriptor type - 2
		VkDescriptorSetLayoutBinding textureLayoutBinding = {};
		textureLayoutBinding.binding = renderer::TEXTURE_DESCRIPTOR_BIND_POINT;
		textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureLayoutBinding.descriptorCount = renderer::r_MAX_TEXTURE_COUNT;
		textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings.push_back(textureLayoutBinding);
		constexpr std::array<VkDescriptorBindingFlags, 2> bindingFlags = {
		    0,
		    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		};
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = bindingFlags.size();
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();


		// Create layout info
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = &bindingFlagsInfo;
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(renderer::r_deviceContext.device, &layoutInfo,
					nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			log::error("vulkan Could not initialize editor's descriptor set layout");
			return false;
		}
		log::info("Initialized editor's descriptor set layout");



		// Creating descriptor pool
		std::vector<VkDescriptorPoolSize> poolSize;

		// Camera UBO
		VkDescriptorPoolSize cameraUBOPoolSize;
		cameraUBOPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraUBOPoolSize.descriptorCount = renderer::r_FRAMES_IN_FLIGHT;
		poolSize.emplace_back(cameraUBOPoolSize);

		// Texture Samplers
		VkDescriptorPoolSize texturePoolSize;
		texturePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturePoolSize.descriptorCount = renderer::r_FRAMES_IN_FLIGHT * renderer::r_MAX_TEXTURE_COUNT;
		poolSize.emplace_back(texturePoolSize);

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolInfo.poolSizeCount = poolSize.size();
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(renderer::r_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(renderer::r_deviceContext.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			clz::log::error("Could not create editor's descriptor pool");
			return false;
		}

		// Creating Descriptor sets
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = renderer::r_FRAMES_IN_FLIGHT;
		const std::vector<VkDescriptorSetLayout> layouts(renderer::r_FRAMES_IN_FLIGHT, descriptorSetLayout);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(renderer::r_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(renderer::r_deviceContext.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		{
			clz::log::error("vulkan could not allocate editor's  descriptor sets");
			return false;
		}

		for (uint32_t i = 0; i < renderer::r_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo cameraBufferInfo = {};
			cameraBufferInfo.buffer = uboMemory.buffer;
			cameraBufferInfo.offset = editorCameraUBO.offsets[i];
			cameraBufferInfo.range = sizeof(EditorShaderUBO);

			std::vector<VkWriteDescriptorSet> descriptorWrites;

			VkWriteDescriptorSet cameraUBOWrite = {};
			cameraUBOWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			cameraUBOWrite.dstSet = descriptorSets[i];
			cameraUBOWrite.dstArrayElement = 0;
			cameraUBOWrite.dstBinding = editorCameraUBO.uboBindingPoint;
			cameraUBOWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			cameraUBOWrite.descriptorCount = 1;
			cameraUBOWrite.pBufferInfo = &cameraBufferInfo;
			descriptorWrites.emplace_back(cameraUBOWrite);

			vkUpdateDescriptorSets(clz::renderer::r_deviceContext.device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
		}

		clz::log::info("(partially) initialised editor's descriptors");
		return true;

	}

	void updateTextureBufferObject()
	{
		if (renderer::r_numRegisteredTextures == 0)
		{
			clz::log::warn("No textures were registered");
			return;
		}
		for (uint32_t i = 0; i < renderer::r_numRegisteredTextures; ++i)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = renderer::r_textures.imageView[i];
			imageInfo.sampler = renderer::r_sampler;

			for (uint32_t j = 0; j < renderer::r_FRAMES_IN_FLIGHT; ++j)
			{
				VkWriteDescriptorSet write{};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = descriptorSets[j];
				write.dstBinding = renderer::TEXTURE_DESCRIPTOR_BIND_POINT;
				write.dstArrayElement = i;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = 1;
				write.pImageInfo = &imageInfo;
				vkUpdateDescriptorSets(clz::renderer::r_deviceContext.device, 1, &write, 0, nullptr);

			}

		}

		clz::log::info("initialized editor's  texture descriptors");

	}

	void destroyDescriptors()
	{
		vkDestroyDescriptorPool(renderer::r_deviceContext.device, descriptorPool, nullptr);
		renderer::destroyUniformBuffer(uboMemory);
		vkDestroyDescriptorSetLayout(renderer::r_deviceContext.device, descriptorSetLayout, nullptr);
		clz::log::info("destroyed editor's descriptor");
	}

}
*/