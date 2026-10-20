/**
 * @file ssbo.hpp
 * @author curl0z
 * @brief Generic per-frame storage buffer helper.
 *
 * Mirrors ubo.hpp's design but for VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
 * buffers — used for variable-length GPU data like the point light
 * array, where a fixed-size UBO wouldn't fit an arbitrary light count.
 */

#pragma once

#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>
#include "../arena.hpp"

namespace clz::renderer
{
	///< @brief Storage buffer creation info.
	typedef struct ssboCreateInfo
	{
		///< Size in bytes of a single frame's worth of data.
		VkDeviceSize size;

		///< Descriptor binding point this SSBO is bound to.
		uint32_t bindPoint;
	} StorageBufferCreateInfo;


	/// @brief Engine's internal Storage buffer handle.
	struct SSBO
	{
		///< mapped[i] = base mappedMemory + offsets[i].
		std::vector<void*> mapped;	  

		///< offsets of each frame's data in memory.
		std::vector<VkDeviceSize> offsets;

		///< Size of a single frame's data.
		VkDeviceSize ssboSize;

		///< Descriptor binding point this UBO is bound to.
		uint32_t ssboBindingPoint;

		///< Single buffer covering all frames-in-flight regions.
		VkBuffer buffer = VK_NULL_HANDLE;

		///< SSBO memory
		clzSSBOMemory memory;
	};

	/**
	 * @brief Creates a storage buffer.
	 * @param ssboCreateInfo Storage buffer creation info struct.
	 * @param rSSBO reference to the 'to be created' ssbo.
	 * @param bufferName Debug name applied to the Vulkan objects.
	 * @return true on success, false if buffer creation failed.
	 */
	bool createStorageBuffer(
		const StorageBufferCreateInfo& ssboCreateInfo,
		SSBO& rSSBO,
		const std::string& bufferName
	);

	/**
	 * @brief Destroys a storage buffer
	 * @param rSSBO Reference of ssbo to destroy.
	 * @param explicitDestroy Flag which specifies whether to destroy
	 * all ubo resources on the spot. Is kind of slow, so false by default.
	 * Dw will be destroyed automatically.
	 */
	void destroyStorageBuffer(SSBO& rSSBO, const bool explicitDestroy = false);
} // namespace clz::renderer
