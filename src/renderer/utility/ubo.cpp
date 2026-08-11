/**
 * @file ubo.cpp
 * @author curl0z
 * @brief Implementation of the per-frame uniform buffer helper (see ubo.hpp).
 */

#include "renderer/utility/ubo.hpp"
#include "core/logs.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/offsetalignment.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{

	/// @copydoc createUniformBuffer
	bool createUniformBuffer(UBO& rUBO, UBOMemory& rUBOMemory, const std::string& bufferName)
	{
		// Lay out one region per frame-in-flight, respecting the device's
		// minimum UBO offset alignment between regions.
		VkDeviceSize totalUBOMemory = 0;
		rUBO.offsets.resize(r_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; i++)
		{
			rUBO.offsets[i] = totalUBOMemory;
			totalUBOMemory = getNextUBOOffsetAlignment(totalUBOMemory + rUBO.uboSize);
		}
		rUBOMemory.memorySize = totalUBOMemory;

		// Single host-visible/coherent buffer big enough for every frame's
		// region — coherent means no explicit flush is needed after memcpy.
		if (!createBuffer(
			    rUBOMemory.buffer,
			    rUBOMemory.memory,
			    bufferName,
			    totalUBOMemory,
			    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		    ))
		{
			clz::log::error("Could not create uniform buffers: " + bufferName);
			return false;
		}
		// Persistently mapped for the buffer's whole lifetime — avoids
		// re-mapping every frame just to write new data in.
		vkMapMemory(
			r_deviceContext.device,
			rUBOMemory.memory,
			0,
			totalUBOMemory,
			0,
			&rUBOMemory.mappedMemory
		);
		setHandleName(
			reinterpret_cast<uint64_t>(rUBOMemory.buffer),
			VK_OBJECT_TYPE_BUFFER,
			bufferName.c_str()
		);
		setHandleName(
			reinterpret_cast<uint64_t>(rUBOMemory.memory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			(bufferName + " memory").c_str()
		);

		// Cache a ready-to-memcpy pointer per frame so callers never need
		// to do the offset arithmetic themselves.
		rUBO.mapped.resize(r_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			rUBO.mapped[i] =
				static_cast<std::byte*>(rUBOMemory.mappedMemory) + rUBO.offsets[i];
		}

		return true;
	}

	/// @copydoc destroyUniformBuffer
	void destroyUniformBuffer(UBO& rUBO, UBOMemory& rUBOMemory)
	{
		vkDestroyBuffer(r_deviceContext.device, rUBOMemory.buffer, nullptr);
		vkFreeMemory(r_deviceContext.device, rUBOMemory.memory, nullptr);
		rUBOMemory.mappedMemory = nullptr;

		rUBO.offsets.clear();
		rUBO.mapped.clear();
	}

} // namespace clz::renderer