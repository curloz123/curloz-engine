/**
 * @file ssbo.cpp
 * @author curl0z
 * @brief Implementation of the per-frame storage buffer helper (see ssbo.hpp).
 */

#include "renderer/utility/ssbo.hpp"
#include "core/logs.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	/// @copydoc createUniformBuffer
	bool createStorageBuffer(
		const StorageBufferCreateInfo& ssboCreateInfo,
		SSBO& rSSBO,
		const std::string& bufferName
	)
	{
		static const auto MinSSBOAlignmentSize = [&]() {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(
				r_deviceContext.physicalDevice,
				&properties
			);
			return properties.limits.minStorageBufferOffsetAlignment;
		}();
		auto getNearestOffset = [alignment = MinSSBOAlignmentSize](
			VkDeviceSize prvOffset)
			{
				return (alignment + prvOffset - 1) &
					~(alignment - 1);
			};

		VkDeviceSize totalSSBOMemory = 0;
		rSSBO.offsets.resize(r_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; i++)
		{
			rSSBO.offsets[i] = totalSSBOMemory;
			totalSSBOMemory = getNearestOffset(totalSSBOMemory + ssboCreateInfo.size);
		}

		const auto& device = clz::renderer::r_deviceContext.device;
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = totalSSBOMemory;
		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(
			device,
			&bufferInfo,
			nullptr,
			&rSSBO.buffer
		) != VK_SUCCESS)
		{
			clz::log::error(
				"vulkan failed to create storage buffer: " + bufferName,
				true
			);
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, rSSBO.buffer, &memRequirements);
		SSBOMemoryAllocInfo memAllocInfo{
			.size = memRequirements.size,
			.alignment = memRequirements.alignment
		};
		if (!carveSSBOMemory(memAllocInfo, rSSBO.memory))
		{
			clz::log::error(
				"could not carve memory for ubo: " + bufferName,
				true
			);
		}

		setHandleName(
			reinterpret_cast<uint64_t>(rSSBO.buffer),
			VK_OBJECT_TYPE_BUFFER,
			bufferName.c_str()
		);

		std::byte* mapped = getSSBOMappedPointer(rSSBO.memory);

		rSSBO.mapped.resize(r_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			rSSBO.mapped[i] = reinterpret_cast<void*>(mapped + rSSBO.offsets[i]);
		}

		rSSBO.ssboSize = ssboCreateInfo.size;

		return true;
	}

	/// @copydoc destroyStorageBuffer
	void destroyStorageBuffer(SSBO& rSSBO, const bool explicitDestroy)
	{
		rSSBO.mapped.clear();
		vkDestroyBuffer(r_deviceContext.device, rSSBO.buffer, nullptr);
		if (explicitDestroy)
			freeSSBOMemory(rSSBO.memory);
	}
} // namespace clz::renderer
