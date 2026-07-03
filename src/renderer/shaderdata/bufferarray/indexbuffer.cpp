/**
 * @file indexbuffer.cpp
 * @author curl0z
 * @brief UV Buffer implementation
 */

#include "renderer/shaderdata/bufferarray/indexbuffer.hpp"
#include "core/logs.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/vk_types.hpp"
#include <memory.h>

namespace clz::renderer::IBuffer
{
	std::tuple<uint32_t, uint32_t> registerIndices(const std::vector<uint32_t>& indices)
	{
		uint32_t startIndex = r_globalIndexVector.size();
		uint32_t indexCount = indices.size();
		r_globalIndexVector.insert(r_globalIndexVector.end(), indices.begin(), indices.end());

		return std::make_tuple(startIndex, indexCount);
	}

	bool submitIndexBuffer()
	{
		if (r_globalIndexVector.empty())
		{
			clz::log::warn("No indices were registered");
			return true;
		}
		const auto& device = clz::renderer::r_deviceContext.device;

		const VkDeviceSize bufferSize = sizeof(r_globalIndexVector[0]) * r_globalIndexVector.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		if (!createBuffer(stagingBuffer, stagingBufferMemory, "Main index staging buffer", bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			clz::log::error("Could not create main index buffer");
			return false;
		}

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, r_globalIndexVector.data(), bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		if (!createBuffer(r_indexBuffer, r_indexBufferMemory, "main index buffer", bufferSize,
				  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		{
			clz::log::error("Could not create main index buffer");
			return false;
		}

		if (!copyBuffer(stagingBuffer, r_indexBuffer, bufferSize, "index staging buffer", "main index buffer"))
		{
			clz::log::error("Could not create main index buffer");
			return false;
		}

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		clz::log::info("Created main index buffer");
		return true;
	}

	void destroyIndexBuffer()
	{
		const auto& device = clz::renderer::r_deviceContext.device;
		vkDestroyBuffer(device, r_indexBuffer, nullptr);
		vkFreeMemory(device, r_indexBufferMemory, nullptr);
	}
} // namespace clz::renderer::IBuffer
