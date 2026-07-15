#include "renderer/utility/ubo.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/utility/offsetalignment.hpp"
#include "renderer/utility/buffer.hpp"
#include "core/logs.hpp"


namespace clz::renderer
{
	bool createUniformBuffers(const std::vector<UBO*>& uniformBuffers, UBOMemory& uboMemory)
	{
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
				totalUBOMemory = getNextUBOOffsetAlignment(totalUBOMemory + uniformBuffers[j]->uboSize);
			}
		}
		uboMemory.memorySize = totalUBOMemory;

		if (!createBuffer(uboMemory.buffer, uboMemory.memory, "IDN which UBO it is",
					totalUBOMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			clz::log::error("Could not create uniform buffers!");
			return false;
		}
		vkMapMemory(r_deviceContext.device, uboMemory.memory,
				0, totalUBOMemory, 0, &uboMemory.mappedMemory);

		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			for (uint32_t j = 0; j < uniformBuffers.size(); ++j)
			{
				uniformBuffers[j]->mapped[i] = static_cast<std::byte*>(uboMemory.mappedMemory) + uniformBuffers[j]->offsets[i];
			}
		}

		return true;
	}

	void destroyUniformBuffer(const UBOMemory& uboMemory)
	{
		vkDestroyBuffer(r_deviceContext.device, uboMemory.buffer, nullptr);
		vkFreeMemory(r_deviceContext.device, uboMemory.memory, nullptr);
	}
}