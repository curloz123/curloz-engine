/**
 * @file indexbuffer.hpp
 * @author curl0z
 * @brief Global GPU index buffer management for the renderer.
 *
 * Stores indices belonging to all loaded meshes inside a single contiguous
 * index array and exposes utilities for uploading the data to GPU memory.
 *
 * Each mesh stores its first index and index count inside this global buffer,
 * allowing multiple meshes to share a single Vulkan index buffer.
 */
#pragma once

#include <cstdint>
#include <tuple>
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	/**
	 * @brief CPU-side storage for all mesh indices.
	 *
	 * Indices are appended as models are loaded and later uploaded
	 * into a single GPU index buffer.
	 */
	inline std::vector<uint32_t> r_globalIndexVector;

	/**
	 * @brief Device local Vulkan index buffer.
	 */
	inline VkBuffer r_indexBuffer;

	/**
	 * @brief Memory backing the global index buffer.
	 */
	inline VkDeviceMemory r_indexBufferMemory;

	/**
	 * @brief Registers mesh indices inside the global index array.
	 *
	 * Appends the supplied indices to the global index vector and returns
	 * the offset and number of inserted indices. The returned values should
	 * be stored by the mesh and later supplied to vkCmdDrawIndexed().
	 *
	 * @param indices Indices belonging to the mesh being loaded.
	 *
	 * @return Tuple containing:
	 *         - First index of the mesh inside the global index buffer.
	 *         - Number of indices belonging to the mesh.
	 */
	std::tuple<uint32_t, uint32_t> registerIndices(const std::vector<uint32_t>& indices);

	/**
	 * @brief Uploads the global index array to GPU memory.
	 *
	 * Creates a host-visible staging buffer, copies index data into it,
	 * and transfers the data into a device local index buffer suitable
	 * for indexed rendering.
	 *
	 * @return True if upload succeeded.
	 */
	bool createIndexBuffer();

	/**
	 * @brief Destroys GPU resources associated with the index buffer.
	 */
	void destroyIndexBuffer();
} // namespace clz::renderer
