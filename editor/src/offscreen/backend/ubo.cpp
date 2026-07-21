#include "../../../include/offscreen/backend/ubo.hpp"
#include "renderer/utility/offsetalignment.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/namer.hpp"
#include "core/logs.hpp"
#include <string>


namespace clz::editor::backend
{
	bool createEditorUniformBuffers()
	{
		// --- 1. Create All uniform buffers
		const std::vector<renderer::UBO*> uniformBuffers = {&editorCameraUBO};

		for (auto* ubo : uniformBuffers)
		{
			ubo->offsets.resize(renderer::r_FRAMES_IN_FLIGHT);
			ubo->mapped.resize(renderer::r_FRAMES_IN_FLIGHT);
		}

		VkDeviceSize totalUBOMemory = 0;
		for (uint32_t i = 0; i < renderer::r_FRAMES_IN_FLIGHT; i++)
		{
			for (uint32_t j = 0; j < uniformBuffers.size(); ++j)
			{
				uniformBuffers[j]->offsets[i] = totalUBOMemory;
				totalUBOMemory = renderer::getNextUBOOffsetAlignment(totalUBOMemory + uniformBuffers[j]->uboSize);
			}
		}
		uboMemory.memorySize = totalUBOMemory;

		const std::string bufferName = "editor Uniform buffer";
		const std::string bufferMemoryName = bufferName + " memory";
		if (!renderer::createBuffer(uboMemory.buffer, uboMemory.memory, bufferName,
					totalUBOMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			clz::log::error("Could not create editor uniform buffers!");
			return false;
		}
		vkMapMemory(renderer::r_deviceContext.device, uboMemory.memory,
				0, totalUBOMemory, 0, &uboMemory.mappedMemory);

		renderer::setHandleName(reinterpret_cast<uint64_t>(uboMemory.buffer), VK_OBJECT_TYPE_BUFFER, bufferName.c_str());
		renderer::setHandleName(reinterpret_cast<uint64_t>(uboMemory.memory), VK_OBJECT_TYPE_DEVICE_MEMORY, bufferMemoryName.c_str());

		for (uint32_t i = 0; i < renderer::r_FRAMES_IN_FLIGHT; ++i)
		{
			for (uint32_t j = 0; j < uniformBuffers.size(); ++j)
			{
				uniformBuffers[j]->mapped[i] = static_cast<std::byte*>(uboMemory.mappedMemory) + uniformBuffers[j]->offsets[i];
			}
		}

		return true;
	}

	void destroyEditorUniformBuffers()
	{
		vkDestroyBuffer(renderer::r_deviceContext.device, uboMemory.buffer, nullptr);
		vkFreeMemory(renderer::r_deviceContext.device, uboMemory.memory, nullptr);
	}
}