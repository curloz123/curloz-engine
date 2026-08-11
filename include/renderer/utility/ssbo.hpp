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

namespace clz::renderer
{

	/**
	 * @brief Raw Vulkan handles backing an SSBO.
	 * @see UBOMemory — same role, storage-buffer variant.
	 */
	struct SSBOMemory
	{
		VkBuffer buffer = VK_NULL_HANDLE;	 ///< Single buffer covering all frames-in-flight regions.
		VkDeviceMemory memory = VK_NULL_HANDLE; ///< Backing device memory for @c buffer.
		void* mappedMemory = nullptr;		 ///< Persistently mapped pointer to the whole buffer.
		VkDeviceSize memorySize = 0;		 ///< Total size of @c buffer, across all frame regions.

		SSBOMemory() = default;
	};

	/**
	 * @brief Logical view over an SSBOMemory — per-frame offsets and
	 * mapped-pointer shortcuts.
	 * @see UBO — same role, storage-buffer variant.
	 */
	struct SSBO
	{
		VkDeviceSize ssboSize;		    ///< Size in bytes of a single frame's worth of data.
		uint32_t ssboBindingPoint;	    ///< Descriptor binding point this SSBO is bound to.
		std::vector<VkDeviceSize> offsets; ///< Byte offset of each frame's region within the buffer.
		std::vector<void*> mapped;	    ///< mapped[i] = base mappedMemory + offsets[i], one per frame in flight.

		/**
		 * @brief Constructs an SSBO descriptor (does not allocate GPU memory).
		 * @param ssboSize Size in bytes needed per frame's copy of the data.
		 * @param ssboBindingPoint Descriptor set binding index for this SSBO.
		 * @note Call createStorageBuffer afterward to actually allocate and
		 * map the backing buffer.
		 */
		explicit SSBO(const VkDeviceSize ssboSize, const uint32_t ssboBindingPoint)
		    : ssboSize(ssboSize), ssboBindingPoint(ssboBindingPoint), offsets({}),
		      mapped({})
		{
		}
	};

	/**
	 * @brief Allocates, maps, and names the Vulkan buffer backing @p rSSBO.
	 * @see createUniformBuffer — identical behaviour, storage-buffer variant.
	 * @param rSSBO Logical SSBO descriptor; its offsets/mapped are populated.
	 * @param rSSBOMemory Output — receives the created buffer/memory handles.
	 * @param bufferName Debug name applied to the Vulkan objects.
	 * @return true on success, false if buffer creation failed.
	 */
	bool createStorageBuffer(SSBO& rSSBO, SSBOMemory& rSSBOMemory, std::string_view bufferName);

	/**
	 * @brief Destroys the buffer/memory backing @p rSSBOMemory and clears
	 * @p rSSBO's per-frame bookkeeping.
	 * @param rSSBO SSBO whose offsets/mapped vectors get cleared.
	 * @param rSSBOMemory Buffer/memory to destroy and free.
	 */
	void destroyStorageBuffer(SSBO& rSSBO, SSBOMemory& rSSBOMemory);

} // namespace clz::renderer