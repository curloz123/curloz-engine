/**
 * @file uvbuffer.cpp
 * @author curl0z
 * @brief UV Buffer implementation
 */

#include "../../../include/renderer/entitydata/uvbuffer.hpp"
#include "core/logs.hpp"
#include "math/vec2.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include <memory.h>

namespace clz::renderer
{
uint32_t getUVBaseIndex()
{
	return r_globalUVArray.size();
}

void registerUVs(const std::vector<clz::math::vec2>& UVs)
{
	r_globalUVArray.insert(r_globalUVArray.end(), UVs.begin(), UVs.end());
}

bool createUVBuffer()
{
	if (r_globalUVArray.empty())
	{
		clz::log::warn("No UVs were registered");
		return true;
	}
	const auto& device = clz::renderer::r_deviceContext.device;

	const VkDeviceSize bufferSize = sizeof(r_globalUVArray[0]) * r_globalUVArray.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	if (!createBuffer(
		    stagingBuffer,
		    stagingBufferMemory,
		    "Main UV staging buffer",
		    bufferSize,
		    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	    ))
	{
		clz::log::error("Could not create main UV buffer");
		return false;
	}

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, r_globalUVArray.data(), bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	if (!createBuffer(
		    r_uvBuffer,
		    r_uvBufferMemory,
		    "main UV buffer",
		    bufferSize,
		    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	    ))
	{
		clz::log::error("Could not create main UV buffer");
		return false;
	}
	setHandleName(
		reinterpret_cast<uint64_t>(r_uvBuffer),
		VK_OBJECT_TYPE_BUFFER,
		"entity UV buffer"
	);
	setHandleName(
		reinterpret_cast<uint64_t>(r_uvBufferMemory),
		VK_OBJECT_TYPE_DEVICE_MEMORY,
		"entity UV buffer memory"
	);

	if (!copyBuffer(
		    stagingBuffer,
		    r_uvBuffer,
		    bufferSize,
		    "UV staging buffer",
		    "UV main buffer"
	    ))
	{
		clz::log::error("Could not create main UV buffer");
		return false;
	}

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	clz::log::info("Created main UV buffer");
	return true;
}

void destroyUVBuffer()
{
	const auto& device = clz::renderer::r_deviceContext.device;
	vkDestroyBuffer(device, r_uvBuffer, nullptr);
	vkFreeMemory(device, r_uvBufferMemory, nullptr);
	clz::log::info("Destroyed UV buffer");
}

VkVertexInputBindingDescription getUVBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 1;
	bindingDescription.stride = sizeof(clz::math::vec2);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

VkVertexInputAttributeDescription getUVAttributeDescription()
{
	VkVertexInputAttributeDescription attributeDescription{};
	attributeDescription.binding = 1;
	attributeDescription.location = 1;
	attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescription.offset = 0;

	return attributeDescription;
}

} // namespace clz::renderer
