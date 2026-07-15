#pragma once

#include "renderer/utility/descriptor.hpp"
#include "math/mat4x4.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include <memory.h>

namespace clz::renderer::ShapePipeline
{

	/// @brief Descriptor's section
	inline UBOMemory uboMemory;

	/// @brief Uniform buffers

	struct CameraShaderUBO
	{
		clz::math::mat4 projection;
		clz::math::mat4 view;
	};

	constexpr uint32_t CAMERA_DESCRIPTOR_BINDING = 1;
	inline UBO cameraUBO(sizeof(CameraShaderUBO), CAMERA_DESCRIPTOR_BINDING);

	/// @brief enable textures?
	constexpr bool enableTextures = false;

	inline void updateCameraUBO(const math::mat4& projection, const math::mat4& view)
	{
		CameraShaderUBO ubo{
			.projection = projection,
			.view = view
		};
		memcpy(cameraUBO.mapped[r_currentFrame], &ubo, sizeof(CameraShaderUBO));
	}


	inline bool createDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		// Descriptor type - 1
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = CAMERA_DESCRIPTOR_BINDING;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings.push_back(uboLayoutBinding);



		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(clz::renderer::r_deviceContext.device, &layoutInfo,
			nullptr, &r_shapePipelineContext.descriptorSetLayout) != VK_SUCCESS)
		{
			clz::log::error("vulkan Could not initialize descriptor set layout");
			return false;
		}
		clz::log::info("Initialized descriptor set layout");
		return true;
	}

}
