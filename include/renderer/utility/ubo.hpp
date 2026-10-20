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

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	///< @brief Uniform buffer creation info.
	typedef struct uboCreateInfo
	{
		///< Size in bytes of a single frame's worth of data.
		VkDeviceSize size;

		///< Descriptor binding point this UBO is bound to.
		std::uint32_t bindPoint;
	} UniformBufferCreateInfo;


	/// @brief Engine's internal Uniform buffer handle.
	struct UBO
	{
		///< mapped[i] = base mappedMemory + offsets[i].
		std::vector<void*> mapped;	  

		///< offsets of each frame's data in memory.
		std::vector<VkDeviceSize> offsets;

		///< Size of a single frame's data.
		VkDeviceSize uboSize;

		///< Descriptor binding point this UBO is bound to.
		uint32_t uboBindingPoint;

		///< Single buffer covering all frames-in-flight regions.
		VkBuffer buffer = VK_NULL_HANDLE;

		///< UBO memory
		clzUBOMemory memory;
	};

	/**
	 * @brief Creates a uniform buffer.
	 * @param uboCreateInfo Uniform buffer creation info struct.
	 * @param rUBO reference to the 'to be created' ubo.
	 * @param bufferName Debug name applied to the Vulkan objects.
	 * @return true on success, false if buffer creation failed.
	 */
	bool createUniformBuffer(
		const UniformBufferCreateInfo& uboCreateInfo,
		UBO& rUBO,
		const std::string& bufferName
	);

	/**
	 * @brief Destroys a uniform buffer
	 * @param rUBO Reference of ubo to destroy.
	 * @param explicitDestroy Flag which specifies whether to destroy
	 * all ubo resources on the spot. Is kind of slow, so false by default.
	 * Dw will be destroyed automatically.
	 */
	void destroyUniformBuffer(UBO& rUBO, const bool explicitDestroy = false);

} // namespace clz::renderer
