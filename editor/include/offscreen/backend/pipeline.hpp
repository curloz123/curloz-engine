#pragma once

#include "renderer/context/context.hpp"
#include "math/mat4x4.hpp"
#include "core/logs.hpp"
#include "renderer/utility/ubo.hpp"
#include "renderer/pipelineinput/globalinput.hpp"
#include "renderer/entitydata/texture.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"

namespace clz::editor::backend
{
	inline renderer::PipelineContext editorPipelineContext{};

	inline renderer::UBOMemory uboMemory{};

	struct CameraShaderUBO
	{
		math::mat4 projection;
		math::mat4 view;
	};
	constexpr uint32_t CAMERA_BINDING_POINT = 1;
	inline renderer::UBO CameraUBO(sizeof(CameraShaderUBO), CAMERA_BINDING_POINT);

	inline bool enableTextures = true;

	inline bool createDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		// Descriptor type - 1
		VkDescriptorSetLayoutBinding textureLayoutBinding = {};
		textureLayoutBinding.binding = renderer::TEXTURE_DESCRIPTOR_BIND_POINT;
		textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureLayoutBinding.descriptorCount = renderer::r_MAX_TEXTURE_COUNT;
		textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings.push_back(textureLayoutBinding);


		// Descriptor type - 2
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = CAMERA_BINDING_POINT;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings.push_back(uboLayoutBinding);


		constexpr std::array<VkDescriptorBindingFlags, 2> bindingFlags = {
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, // For texture
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
			nullptr, &editorPipelineContext.descriptorSetLayout) != VK_SUCCESS)
		{
			clz::log::error("vulkan Could not initialize descriptor set layout");
			return false;
		}
		if (clz::log::errorOccurred())
		{
			return false;
		}
		clz::log::info("Initialized descriptor set layout");
		return true;

	}
}

namespace clz::editor::backend
{
	bool initializeEditorPipeline();
	void destroyEditorPipeline();
	void updateTextureBufferObject();
}