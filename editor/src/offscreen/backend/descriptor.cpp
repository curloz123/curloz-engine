#include "../../../include/offscreen/backend/descriptor.hpp"
#include "../../../include/offscreen/backend/ubo.hpp"
#include "core/logs.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include <vector>
#include <vulkan/vulkan.h>
#include <string>

namespace clz::editor::backend
{
bool createEditorDescriptors()
{
	std::vector<VkDescriptorPoolSize> poolSize;

	// UBOs (camera, and any future ones counted via NUM_UNIFORM_BUFFERS)
	VkDescriptorPoolSize UBOPoolSize = {};
	UBOPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	UBOPoolSize.descriptorCount =
		renderer::r_FRAMES_IN_FLIGHT * NUM_BACKEND_UNIFORM_BUFFERS;
	poolSize.emplace_back(UBOPoolSize);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
	poolInfo.pPoolSizes = poolSize.data();
	poolInfo.maxSets = static_cast<uint32_t>(renderer::r_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(
		    renderer::r_deviceContext.device,
		    &poolInfo,
		    nullptr,
		    &descriptorPool
	    ) != VK_SUCCESS)
	{
		clz::log::error("Could not create descriptor pool");
		return false;
	}
	renderer::setHandleName(
		reinterpret_cast<uint64_t>(descriptorPool),
		VK_OBJECT_TYPE_DESCRIPTOR_POOL,
		"editor descriptor pool"
	);

	// --- Allocate camera UBO descriptor sets, one per frame-in-flight ---
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = renderer::r_FRAMES_IN_FLIGHT;

		const std::vector editorDescriptorLayouts(
			renderer::r_FRAMES_IN_FLIGHT,
			renderer::cameraDescriptorLayout
		);
		allocInfo.pSetLayouts = editorDescriptorLayouts.data();

		cameraDescriptorSets.resize(renderer::r_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(
			    renderer::r_deviceContext.device,
			    &allocInfo,
			    cameraDescriptorSets.data()
		    ) != VK_SUCCESS)
		{
			clz::log::error("vulkan could not allocate camera descriptor sets");
			return false;
		}

		for (uint32_t i = 0; i < renderer::r_FRAMES_IN_FLIGHT; ++i)
		{
			renderer::setHandleName(
				reinterpret_cast<uint64_t>(cameraDescriptorSets[i]),
				VK_OBJECT_TYPE_DESCRIPTOR_SET,
				("editor camera descriptor set " + std::to_string(i)).c_str()
			);
		}
	}

	// --- Write the camera UBO's buffer binding into each of its descriptor sets ---
	// (Sampler bindings are written later via updateSamplersDataForDescriptorSets,
	//  once scene textures actually exist to point at.)
	for (uint32_t i = 0; i < renderer::r_FRAMES_IN_FLIGHT; ++i)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uboMemory.buffer;
		bufferInfo.offset = editorCameraUBO.offsets[i];
		bufferInfo.range = editorCameraUBO.uboSize;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = cameraDescriptorSets[i];
		write.dstBinding = editorCameraUBO.uboBindingPoint;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(
			renderer::r_deviceContext.device,
			1,
			&write,
			0,
			nullptr
		);
	}

	return true;
}

void destroyEditorDescriptors()
{
	vkDestroyDescriptorPool(renderer::r_deviceContext.device, descriptorPool, nullptr);
}
} // namespace clz::editor::backend
