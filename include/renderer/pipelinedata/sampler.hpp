/**
 * @file sampler.hpp
 * @author curl0z
 * @brief Create pipeline's combined sampler storage
 * that will hold all textures in the gpu.
 * The file's layout is similar -
 * 1. Declare data
 * 2. Declare functions
 *
 * @note This may sound similar to textures,
 * but since texture suits more with entity data, most of the
 * implementation stays there. This file only contains
 * The descriptor set layouts and bind points.
 * @note update samplers data for your
 * targeted descriptor after scene have been loaded.
 */
#pragma once

#include <vector>
#include <vulkan/vulkan.h>

/// --- Data ----
namespace clz::renderer
{
	/// @brief Combined sampler / texture bind point
	/// Is global, shall not be used by anything else
	/// @note bind poit might be equal to others,
	/// but since their sets are different, it's not a problem
	inline constexpr uint8_t COMBINED_SAMPLER_BINDPOINT = 0;
	/// @brief combiner sampler layout
	inline VkDescriptorSetLayout combinedSamplerLayout;
} // namespace clz::renderer

/// --- Functions ---
namespace clz::renderer
{
	/// @brief Creates combined image samplers layout
	/// @return true on success, else ukw
	bool createCombinedSamplersLayout();

	/// @brief This function is the one that actually writes
	/// data back to the gpu.
	/// @param descriptorSets array of "already initialized" descriptor sets
	/// make sure the size is FRAMES_IN_FLIGHT
	/// @warning MAKE SURE TO CALL IT, and call it after scene have been loaded
	/// precisely in updatePipelineData
	void updateSamplersDataForDescriptorSets(const std::vector<VkDescriptorSet>& descriptorSets);

	/// @brief Destroys combined image samplers
	void destroyCombinedImageSamplersLayout();
} // namespace clz::renderer