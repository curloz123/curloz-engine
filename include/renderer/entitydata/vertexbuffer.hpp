/**
 * @file vertexbuffer.hpp
 * @author curl0z
 * @brief Global GPU vertex buffer management for the renderer.
 *
 * Stores all vertex positions uploaded by loaded meshes inside a single
 * contiguous vertex array and exposes utilities for uploading the data
 * to GPU memory and describing its layout to Vulkan.
 */
#pragma once

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{

	enum class VertexAttributeType
	{
		POSITION,
		UV,
		NORMAL
	};
	struct VertexAttribute
	{
		math::vec3 position;
		math::vec2 uv;
		math::vec3 normal;
	};
	/**
	 * @brief CPU-side storage for all registered mesh vertices.
	 *
	 * Vertices are appended as models are loaded and later uploaded
	 * into a single GPU vertex buffer.
	 */
	inline std::vector<VertexAttribute> r_globalVertexVector;

	/**
	 * @brief Device local Vulkan vertex buffer containing all vertices.
	 */
	inline VkBuffer r_vertexBuffer;

	/**
	 * @brief Memory backing the main vertex buffer.
	 */
	inline VkDeviceMemory r_vertexBufferMemory;

	/**
	 * @brief Returns the starting vertex index for the next mesh.
	 *
	 * This value should be stored as the mesh's base vertex and later
	 * supplied to vkCmdDrawIndexed().
	 *
	 * @return Index of the first vertex that will be inserted next.
	 */
	uint32_t getVertexBaseIndex();

	/**
	 * @brief Appends vertices to the global vertex array.
	 *
	 * @param vertexAttributes Vertex attributes belonging to the mesh being loaded.
	 */
	void registerVertexAttributes(const std::vector<VertexAttribute>& vertexAttributes);

	/**
	 * @brief Uploads the global vertex array to GPU memory.
	 *
	 * Creates a host-visible staging buffer, copies vertex data into it,
	 * and transfers the data into a device local vertex buffer suitable
	 * for rendering.
	 *
	 * @return True if upload succeeded.
	 */
	bool createVertexBuffer();

	/**
	 * @brief Destroys GPU resources associated with the vertex buffer.
	 */
	void destroyVertexBuffer();

	/**
	 * @brief Returns Vulkan binding information for vertex positions.
	 * Describes the stride and input rate for binding slot 0.
	 *
	 * @return Vertex binding description.
	 */
	VkVertexInputBindingDescription getVertexBindingDescription();

	/**
	 * @brief Returns Vulkan attribute description for vertex positions.
	 * Maps binding 0 to shader location 0 using a vec3 floating point
	 * position format.
	 *
	 * @param type Vertex attribute type
	 * @return Vertex attribute description.
	 */
	VkVertexInputAttributeDescription getVertexAttributeDescription(VertexAttributeType type);
} // namespace clz::renderer
