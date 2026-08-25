/**
 * @file ssbo.cpp
 * @author curl0z
 * @brief Implementation of the per-frame storage buffer helper (see ssbo.hpp).
 */

#include "renderer/utility/ssbo.hpp"
#include "core/logs.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/offsetalignment.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{

	/// @copydoc createStorageBuffer
	bool
	createStorageBuffer(SSBO& rSSBO, SSBOMemory& rSSBOMemory, const std::string_view bufferName)
	{
		// Same per-frame region layout strategy as createUniformBuffer,
		// but aligned to storage-buffer offset requirements instead of UBO.
		VkDeviceSize totalSSBOMemory = 0;
		rSSBO.offsets.resize(r_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; i++)
		{
			rSSBO.offsets[i] = totalSSBOMemory;
			totalSSBOMemory =
				getNextSSBOOffsetAlignment(totalSSBOMemory + rSSBO.ssboSize);
		}
		rSSBOMemory.memorySize = totalSSBOMemory;

		if (!createBuffer(
			    rSSBOMemory.buffer,
			    rSSBOMemory.memory,
			    std::string(bufferName),
			    totalSSBOMemory,
			    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		    ))
		{
			clz::log::error(
				"Could not create storage buffer: " + std::string(bufferName)
			);
			return false;
		}
		// Persistently mapped, same rationale as the UBO path.
		vkMapMemory(
			r_deviceContext.device,
			rSSBOMemory.memory,
			0,
			totalSSBOMemory,
			0,
			&rSSBOMemory.mappedMemory
		);
		setHandleName(
			reinterpret_cast<uint64_t>(rSSBOMemory.buffer),
			VK_OBJECT_TYPE_BUFFER,
			bufferName.data()
		);
		setHandleName(
			reinterpret_cast<uint64_t>(rSSBOMemory.memory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			(std::string(bufferName) + " memory").c_str()
		);

		rSSBO.mapped.resize(r_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			rSSBO.mapped[i] = static_cast<std::byte*>(rSSBOMemory.mappedMemory) +
					  rSSBO.offsets[i];
		}

		return true;
	}

	/// @copydoc destroyStorageBuffer
	void destroyStorageBuffer(SSBO& rSSBO, SSBOMemory& rSSBOMemory)
	{
		vkDestroyBuffer(r_deviceContext.device, rSSBOMemory.buffer, nullptr);
		vkFreeMemory(r_deviceContext.device, rSSBOMemory.memory, nullptr);
		rSSBOMemory.mappedMemory = nullptr;

		rSSBO.offsets.clear();
		rSSBO.mapped.clear();
	}

} // namespace clz::renderer