/**
 * @file uvbuffer.hpp
 * @author curl0z
 * @brief Global GPU UV buffer management for the renderer.
 *
 * Stores texture coordinates for all loaded meshes inside a single
 * contiguous UV array and exposes utilities for uploading the data
 * to GPU memory and describing its layout to Vulkan.
 *
 * This buffer occupies vertex binding slot 1 and supplies UV coordinates
 * to shader location 1.
 */
#pragma once

#include "math/vec2.hpp"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer::UVBuffer
{
	/**
	 * @brief CPU-side storage for all registered texture coordinates.
	 *
	 * UV coordinates are appended as models are loaded and later uploaded
	 * into a single GPU vertex buffer.
	 */
	inline std::vector<clz::math::vec2> r_globalUVArray;

	/**
	 * @brief Device local Vulkan buffer containing all UV coordinates.
	 */
	inline VkBuffer r_uvBuffer;

	/**
	 * @brief Memory backing the global UV buffer.
	 */
	inline VkDeviceMemory r_uvBufferMemory;

	/**
	 * @brief Returns the starting UV index for the next mesh.
	 *
	 * This value should be stored by the mesh and used as the offset
	 * when binding the UV vertex buffer.
	 *
	 * @return Index of the first UV coordinate that will be inserted next.
	 */
	uint32_t getBaseVertex();

	/**
	 * @brief Appends UV coordinates to the global UV array.
	 *
	 * @param UVs Texture coordinates belonging to the mesh being loaded.
	 */
	void registerUVs(const std::vector<clz::math::vec2>& UVs);

	/**
	 * @brief Uploads the global UV array to GPU memory.
	 *
	 * Creates a host-visible staging buffer, copies UV data into it,
	 * and transfers the data into a device local vertex buffer suitable
	 * for rendering.
	 *
	 * @return True if upload succeeded.
	 */
	bool submitUVBuffer();

	/**
	 * @brief Destroys GPU resources associated with the UV buffer.
	 */
	void destroyUVBuffer();

	/**
	 * @brief Returns Vulkan binding information for texture coordinates.
	 *
	 * Describes the stride and input rate for vertex binding slot 1.
	 *
	 * @return Vertex binding description for UV data.
	 */
	VkVertexInputBindingDescription getUVBindingDescription();

	/**
	 * @brief Returns Vulkan attribute description for texture coordinates.
	 *
	 * Maps binding slot 1 to shader location 1 using a vec2 floating point
	 * texture coordinate format.
	 *
	 * @return Vertex attribute description for UV data.
	 */
	VkVertexInputAttributeDescription getUVAttributeDescription();
} // namespace clz::renderer::UVBuffer
