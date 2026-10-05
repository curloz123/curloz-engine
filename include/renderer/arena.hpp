/**
 * @file arena.hpp
 * @author curl0z
 * @brief Vulkan memory arena allocator
 */

#pragma once

#include "renderer/utility/memory.hpp"
#include "vk_types.hpp"
#include "utility/buffer.hpp"
#include <vulkan/vulkan_core.h>
#include "core/logs.hpp"

namespace clz::renderer
{
	///< @brief Main vertex buffer memory handle
	inline VkDeviceMemory VertexBufferMemory;

	/**
	 * @brief Allocates vertex buffer memory
	 * @param memRequirements Requirements from memory.
	 * @param memPropertyFlags Allocated memory property flags.
	 * @return bool indicating completion status.
	 */
	inline bool allocateVertexBufferMemory(
		const VkMemoryRequirements& memRequirements,
		const VkMemoryPropertyFlags& memPropertyFlags)
	{
		const auto& device = r_deviceContext.device;
		const VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(
						memRequirements.memoryTypeBits, 
						memPropertyFlags)
		};

		if (vkAllocateMemory(
			device, 
			&allocInfo, 
			nullptr, 
			&VertexBufferMemory) != VK_SUCCESS)
		{
			clz::log::error("Could not allocate vertex buffer memory");
			return false;
		}

		return true;
	}

	/// @brief Destroys vertex buffer memory
	inline void destroyVertexBufferMemory()
	{
		vkFreeMemory(
			r_deviceContext.device,
			VertexBufferMemory, 
			nullptr);
	}


	///< @brief Defines memory creation data	
	typedef struct VkArenaDef
	{
		///< @brief Vertex buffer's memory requirements.
		VkMemoryRequirements vbuffer_mem_requirements;
		///< @brief Vertex buffer's memory's property flags.
		VkMemoryPropertyFlags vbuffer_mem_property_flags;
	
		///< Memory to be allocated for textures.
		VkDeviceSize Texture = 1024 * 1024 * 1024;

		///< Memory to be allocated for UBO's.
		VkDeviceSize UBO = 100 * 1024 * 1024;

		///< Memory to be allocated for SSBO's.
		VkDeviceSize SSBO = 300 * 1024 * 1024;
	} VkArenaDef;

	/**
	 * @brief Allocates vulkan arena's their respective memory.
	 * @param arenaDef Arena's definition.
	 * @return bool indicating success status.
	 */
	inline bool allocateVkArenaMemory(const VkArenaDef& arenaDef)
	{
		if (!allocateVertexBufferMemory(
			arenaDef.vbuffer_mem_requirements,
			arenaDef.vbuffer_mem_property_flags
		))
		{
			clz::log::error("Unable to allocate vertex buffer memory");
			return false;
		}




		return true;
	}
	
	/// @brief Destroy's all arena's memory
	inline void destroyVkArenaMemory()
	{

		destroyVertexBufferMemory();
	}

}
