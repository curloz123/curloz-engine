/**
 * @file uniformbuffers.hpp
 * @author curl0z
 * @brief Uniform buffer allocation and update utilities.
 */

#pragma once

#include "math/mat4x4.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	constexpr uint32_t r_NUM_UNIFORM_BUFFERS = 1;

	struct TransformUBO
	{
		clz::math::mat4 projection;
		clz::math::mat4 view;
	};

} // namespace clz::renderer

namespace clz::renderer
{
	inline VkBuffer r_uniformBuffers;
	inline VkDeviceMemory r_uniformBuffersMemory;
	inline void* r_uniformBuffersMemoryMapped;
	inline VkDeviceSize r_totalUBOMemorySize;

	constexpr uint32_t r_TRANSFORM_DESCRIPTOR_BINDING = 0;
	inline std::vector<VkDeviceSize> r_transformUBOOffsets;
	inline std::vector<void*> r_transformUBOMapped;
} // namespace clz::renderer

namespace clz::renderer
{
	/**
	 * @brief Allocates and maps uniform buffers.
	 *
	 * Creates one aligned TransformUBO region for every frame in flight.
	 *
	 * @return True on success.
	 */
	bool createUniformBuffers();

	/**
	 * @brief Updates uniform data for the current frame.
	 *
	 * Updates view and projection matrices used by shaders.
	 *
	 * @param currentFrame Current frame in flight.
	 */
	void updateUniformBuffers(uint32_t currentFrame);

	/**
	 * @brief Releases all uniform buffer resources.
	 */
	void destroyUniformBuffers();
} // namespace clz::renderer
