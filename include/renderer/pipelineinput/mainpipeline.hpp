#pragma once

#include "renderer/utility/descriptor.hpp"
#include "math/mat4x4.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/pipelineinput/globalinput.hpp"
#include <cstring>
#include "core/logs.hpp"

namespace clz::renderer::MainPipeline
{
	/// @brief Descriptor's section

	inline UBOMemory uboMemory;

	/// @brief Uniform buffers

	/// @brief Camera UBO
	struct CameraShaderUBO
	{
		clz::math::mat4 projection;
		clz::math::mat4 view;
	};
	constexpr uint32_t CAMERA_DESCRIPTOR_BINDING = 1;
	inline UBO cameraUBO(sizeof(CameraShaderUBO), CAMERA_DESCRIPTOR_BINDING);

	/// @brief enable textures?
	constexpr bool enableTextures = true;

	inline void updateCameraUBO(const math::mat4& projection, const math::mat4& view)
	{
		const CameraShaderUBO ubo{
			.projection = projection,
			.view = view,
		};
		memcpy(cameraUBO.mapped[r_currentFrame], &ubo, sizeof(CameraShaderUBO));
	}



	inline bool createDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		// Descriptor type - 1
		VkDescriptorSetLayoutBinding textureLayoutBinding = {};
		textureLayoutBinding.binding = TEXTURE_DESCRIPTOR_BIND_POINT;
		textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureLayoutBinding.descriptorCount = r_MAX_TEXTURE_COUNT;
		textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings.push_back(textureLayoutBinding);


		// Descriptor type - 2
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = CAMERA_DESCRIPTOR_BINDING;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings.push_back(uboLayoutBinding);


		constexpr std::array<VkDescriptorBindingFlags, 2> bindingFlags = {
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, // For textures
			0
		};
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = bindingFlags.size();
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = &bindingFlagsInfo;
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(clz::renderer::r_deviceContext.device, &layoutInfo,
			nullptr, &r_pipelineContext.descriptorSetLayout) != VK_SUCCESS)
		{
			clz::log::error("vulkan Could not initialize descriptor set layout");
			return false;
		}
		clz::log::info("Initialized descriptor set layout");
		return true;
	}
}