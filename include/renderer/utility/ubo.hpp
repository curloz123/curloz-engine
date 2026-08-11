/**
 * @file ubo.hpp
 * @author curl0z
 * @brief Generic per-frame uniform buffer helper.
 *
 * A UBO here is a single logical uniform buffer that's actually backed
 * by one physical VkBuffer sized to hold r_FRAMES_IN_FLIGHT copies —
 * one region per in-flight frame, so the CPU can write next frame's
 * data while the GPU is still reading the previous frame's region.
 */

#pragma once

#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{

	/**
	 * @brief Raw Vulkan handles backing a UBO.
	 *
	 * Kept separate from UBO itself so the handle/memory lifetime can be
	 * managed independently of the logical per-frame offset bookkeeping.
	 */
	struct UBOMemory
	{
		VkBuffer buffer = VK_NULL_HANDLE;	 ///< Single buffer covering all frames-in-flight regions.
		VkDeviceMemory memory = VK_NULL_HANDLE; ///< Backing device memory for @c buffer.
		void* mappedMemory = nullptr;		 ///< Persistently mapped pointer to the whole buffer.
		VkDeviceSize memorySize = 0;		 ///< Total size of @c buffer, across all frame regions.

		UBOMemory() = default;
	};

	/**
	 * @brief Logical view over a UBOMemory — per-frame offsets and
	 * mapped-pointer shortcuts.
	 */
	struct UBO
	{
		VkDeviceSize uboSize;		  ///< Size in bytes of a single frame's worth of data.
		uint32_t uboBindingPoint;	  ///< Descriptor binding point this UBO is bound to.
		std::vector<VkDeviceSize> offsets; ///< Byte offset of each frame's region within the buffer.
		std::vector<void*> mapped;	  ///< mapped[i] = base mappedMemory + offsets[i], one per frame in flight.

		/**
		 * @brief Constructs a UBO descriptor (does not allocate GPU memory).
		 * @param uboSize Size in bytes needed per frame's copy of the data.
		 * @param uboBindingPoint Descriptor set binding index for this UBO.
		 * @note Call createUniformBuffer afterward to actually allocate and
		 * map the backing buffer.
		 */
		explicit UBO(const VkDeviceSize uboSize, const uint32_t uboBindingPoint)
		    : uboSize(uboSize), uboBindingPoint(uboBindingPoint), offsets({}), mapped({})
		{
		}
	};

	/**
	 * @brief Allocates, maps, and names the Vulkan buffer backing @p rUBO.
	 *
	 * Computes per-frame offsets (respecting UBO alignment requirements),
	 * allocates a single host-visible/coherent buffer sized to hold all
	 * r_FRAMES_IN_FLIGHT regions, and persistently maps it.
	 *
	 * @param rUBO Logical UBO descriptor; its offsets/mapped are populated.
	 * @param rUBOMemory Output — receives the created buffer/memory handles.
	 * @param bufferName Debug name applied to the Vulkan objects.
	 * @return true on success, false if buffer creation failed.
	 */
	bool createUniformBuffer(UBO& rUBO, UBOMemory& rUBOMemory, const std::string& bufferName);

	/**
	 * @brief Destroys the buffer/memory backing @p rUBOMemory and clears
	 * @p rUBO's per-frame bookkeeping.
	 * @param rUBO UBO whose offsets/mapped vectors get cleared.
	 * @param rUBOMemory Buffer/memory to destroy and free.
	 */
	void destroyUniformBuffer(UBO& rUBO, UBOMemory& rUBOMemory);

} // namespace clz::renderer