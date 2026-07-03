/**
 * @file vertexbuffer.cpp
 * @author curl0z
 * @brief Implementaion of vertex buffer
 */

#include "renderer/shaderdata/bufferarray/vertexbuffer.hpp"
#include "core/logs.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/vk_types.hpp"
#include <memory.h>

namespace clz::renderer::VBuffer
{
	uint32_t getBaseVertex()
	{
		return r_globalVertexVector.size();
	}

	void registerVertices(const std::vector<clz::math::vec3>& vertices)
	{
		r_globalVertexVector.insert(r_globalVertexVector.end(), vertices.begin(), vertices.end());
	}

	bool submitVertexBuffer()
	{
		if (r_globalVertexVector.empty())
		{
			clz::log::warn("No vertices were registered");
			return true;
		}
		const auto& device = clz::renderer::r_deviceContext.device;

		const VkDeviceSize bufferSize = sizeof(r_globalVertexVector[0]) * r_globalVertexVector.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		if (!createBuffer(stagingBuffer, stagingBufferMemory, "Main vertex staging buffer", bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			clz::log::error("Could not create main vertex buffer");
			return false;
		}

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, r_globalVertexVector.data(), bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		if (!createBuffer(r_vertexBuffer, r_vertexBufferMemory, "main vertex buffer", bufferSize,
				  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		{
			clz::log::error("Could not create main vertex buffer");
			return false;
		}

		if (!copyBuffer(stagingBuffer, r_vertexBuffer, bufferSize, "staging buffer", "main vertex buffer"))
		{
			clz::log::error("Could not create main vertex buffer");
			return false;
		}

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		clz::log::info("Created main vertex buffer");
		return true;
	}

	void destroyVertexBuffer()
	{
		const auto& device = clz::renderer::r_deviceContext.device;
		vkDestroyBuffer(device, r_vertexBuffer, nullptr);
		vkFreeMemory(device, r_vertexBufferMemory, nullptr);
		clz::log::info("Destroyed vertex buffer");
	}

	VkVertexInputBindingDescription getVertexBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(clz::math::vec3);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	VkVertexInputAttributeDescription getVertexAttributeDescription()
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = 0;
		attributeDescription.location = 0;
		attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription.offset = 0;

		return attributeDescription;
	}

} // namespace clz::renderer::VBuffer
