/**
 * @file descriptor.hpp
 * @author curl0z
 *
 * @brief All shader related descriptors
 */
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace clz::renderer
{
	/// Main Descriptor pool
	inline VkDescriptorPool r_descriptorPool;

	/// Descriptor's layout
	inline VkDescriptorSetLayout r_descriptorSetLayout;

	/// Descriptor sets
	inline std::vector<VkDescriptorSet> r_descriptorSets;
}

namespace clz::renderer
{
	/**
	 * @brief Initializes All descriptors like uniform buffer. Texture samplers are not loaded at this step
	 * Also initializes descriptor layout, pool and sets.
	 *
	 * @return true if successful, false otherwise and logs an error
	 */
	bool initDescriptors();

	/**
	 * @brief Creates all descriptors, after all the left-over data
	 * such as texture samplers have been loaded by the entity system
	 *
	 * @return true if successful, false otherwise and logs an error
	 */
	bool createDescriptors();

	/**
	 * @brief Updates all descriptors
	 *
	 * @param commandBuffer Command buffer to record on
	 * @param currentFrame index of current frame
	 */
	void updateDescriptors(VkCommandBuffer commandBuffer, const uint32_t currentFrame);

	/**
	 * @brief Destroys all descriptors
	 */
	void destroyDescriptors();
}
