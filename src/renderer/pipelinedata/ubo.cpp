/**
 * @file ubo.cpp
 * @author curl0z
 * @brief Implementation of Pipeline input's Uniform buffers
 */

#include "renderer/pipelinedata/ubo.hpp"
#include "core/logs.hpp"
#include "memory.h"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/offsetalignment.hpp"

namespace clz::renderer
{
/// @brief Creates all uniform buffers, and creates their pipeline layouts at last
/// Currently creates -
/// 1. Camera UBO
/// @note Must be called only by createPipelineInput();
bool createUniformBuffers()
{
	// --- 1. Create All uniform buffers
	const std::vector<UBO*> uniformBuffers = {&cameraUBO};

	for (auto* ubo : uniformBuffers)
	{
		ubo->offsets.resize(r_FRAMES_IN_FLIGHT);
		ubo->mapped.resize(r_FRAMES_IN_FLIGHT);
	}

	VkDeviceSize totalUBOMemory = 0;
	for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; i++)
	{
		for (uint32_t j = 0; j < uniformBuffers.size(); ++j)
		{
			uniformBuffers[j]->offsets[i] = totalUBOMemory;
			totalUBOMemory = getNextUBOOffsetAlignment(
				totalUBOMemory + uniformBuffers[j]->uboSize
			);
		}
	}
	uboMemory.memorySize = totalUBOMemory;

	const std::string bufferName = "Uniform buffer";
	const std::string bufferMemoryName = bufferName + " memory";
	if (!createBuffer(
		    uboMemory.buffer,
		    uboMemory.memory,
		    bufferName,
		    totalUBOMemory,
		    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	    ))
	{
		clz::log::error("Could not create uniform buffers!");
		return false;
	}
	vkMapMemory(
		r_deviceContext.device,
		uboMemory.memory,
		0,
		totalUBOMemory,
		0,
		&uboMemory.mappedMemory
	);

	setHandleName(
		reinterpret_cast<uint64_t>(uboMemory.buffer),
		VK_OBJECT_TYPE_BUFFER,
		bufferName.c_str()
	);
	setHandleName(
		reinterpret_cast<uint64_t>(uboMemory.memory),
		VK_OBJECT_TYPE_DEVICE_MEMORY,
		bufferMemoryName.c_str()
	);

	for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
	{
		for (uint32_t j = 0; j < uniformBuffers.size(); ++j)
		{
			uniformBuffers[j]->mapped[i] =
				static_cast<std::byte*>(uboMemory.mappedMemory) +
				uniformBuffers[j]->offsets[i];
		}
	}

	/// @brief --- Create their descriptor layouts ---

	const std::vector<VkDescriptorSetLayout*> pDescriptorSetLayouts = {&cameraUBOLayout};
	const std::vector<std::string> descriptorSetLayoutNames = {"camera ubo layout"};
	const std::vector<VkShaderStageFlags> shaderStages = {VK_SHADER_STAGE_VERTEX_BIT};
	const std::vector<uint8_t> bindPoints = {CAMERA_UBO_BINDPOINT};

	for (size_t i = 0; i < pDescriptorSetLayouts.size(); ++i)
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = bindPoints[i];
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = shaderStages[i];

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(
			    r_deviceContext.device,
			    &layoutInfo,
			    nullptr,
			    pDescriptorSetLayouts[i]
		    ) != VK_SUCCESS)
		{
			clz::log::error(
				"vulkan Could not initialize descriptor set layout: " +
				descriptorSetLayoutNames[i]
			);
			return false;
		}

		setHandleName(
			reinterpret_cast<uint64_t>(*(pDescriptorSetLayouts[i])),
			VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
			descriptorSetLayoutNames[i].c_str()
		);
	}

	return true;
}

/// @brief Updates all uniform buffers
/// Currently updates -
/// 1. Camera UBO
/// @note Must be called only once per frame before performing any vkCmdDraw
void updateUniformBuffers(const CameraShaderUBO& shaderCameraUBO)
{
	// --- Camera UBO ---
	memcpy(cameraUBO.mapped[r_currentFrame], &shaderCameraUBO, sizeof(CameraShaderUBO));

	// --- Rest go below ---
}

/// @brief Destroys all uniform buffers (just deletes the main memory)
void destroyUniformBuffers()
{
	vkDestroyDescriptorSetLayout(r_deviceContext.device, cameraUBOLayout, nullptr);
	vkDestroyBuffer(r_deviceContext.device, uboMemory.buffer, nullptr);
	vkFreeMemory(r_deviceContext.device, uboMemory.memory, nullptr);
}
} // namespace clz::renderer