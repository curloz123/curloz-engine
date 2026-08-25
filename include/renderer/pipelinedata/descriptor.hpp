/**
 * @file descriptor.hpp
 * @author curl0z
 * @brief Contains the descriptor pool and per-UBO / per-sampler descriptor
 * sets used by the main render pipeline.
 *
 * @note Each UBO (see ubo.hpp) and the combined image sampler (see
 * sampler.hpp) gets its OWN VkDescriptorSetLayout and its own set of
 * per-frame-in-flight VkDescriptorSets. This keeps them independently
 * bindable (and independently reusable across other pipelines that share
 * a layout-compatible binding, e.g. the editor's offscreen pipeline) rather
 * than being bundled into one monolithic descriptor set.
 */

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	/// @brief Single pool backing every descriptor set allocated below.
	inline VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	/// @brief Camera's descriptor sets
	inline std::vector<VkDescriptorSet> cameraDescriptorSets = {};

	/// @brief Texture's descriptor sets
	inline std::vector<VkDescriptorSet> textureDescriptorSets = {};

	/// @brief Light's descriptor sets
	inline std::vector<VkDescriptorSet> lightDescriptorSets = {};

} // namespace clz::renderer

namespace clz::renderer
{
	/// @brief Create's descriptor layout of all resources
	/// @return True on succesful creation of resources, false otherwise
	bool createDescriptorSetsLayout();

	/// @brief Creates global descriptor pool
	/// @return True on succesful creation of resources, false otherwise
	bool createDescriptorPool();

	/** @brief Creates the descriptor pool, allocates cameraDescriptorSets and
	 *  samplerDescriptorSets from their respective layouts, and writes the
	 *  camera UBO's buffer bindings. Texture/sampler bindings are written
	 *  separately via updateSamplersDataForDescriptorSets, once scene
	 *  textures are loaded.
	 *  @note Requires createUniformBuffers() and createCombinedSamplersLayout()
	 *  to have already run, since their layouts are consumed here.
	 */
	bool allocateDescriptorSets();

	/// @brief Destroys the descriptor pool (which implicitly frees every
	/// descriptor set allocated from it).
	void destroyDescriptorPool();

} // namespace clz::renderer
