/**
 * @file post_process.hpp
 * @author curl0z
 * @brief This file describes all post processes descriptor resources.
 * Each process and its relevant resources have been given their bind points.
 * Also, this descriptor is updated/written only by the post_process system
 *
 * General workflow is, 
 * create render_target_context -> create Post Process System -> Create post_process descriptor while creating descriptor sets
 */
#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	/// @brief Initial render target context's image bind point (0)
	inline constexpr uint8_t RENDER_TARGET_IMAGE_BIND_POINT = 0;
	inline constexpr uint8_t BLOOM_SAMPLE_IMAGE_BIND_POINT = 1;
	/// @brief Bloom Mip-images bind point (2)
	inline constexpr uint8_t BLOOM_MIP_IMAGES_BIND_POINT = 2;
	/// @brief Final Bloomed image bind point (3)
	inline constexpr uint8_t BLOOMED_IMAGE_BIND_POINT = 3;
	/// @brief Final Tonemapped image bind point (4)
	inline constexpr uint8_t TONEMAP_IMAGE_BIND_POINT = 4;
	/// @brief post tonemapped image bind point (5)
	inline constexpr uint8_t POST_TONEMAP_IMAGE_BIND_POINT = 5;

	/// @brief Post process descriptor set layout
	inline VkDescriptorSetLayout post_processDescriptorLayout;

	/// @brief Post process descriptor sets
	inline std::array<VkDescriptorSet, r_FRAMES_IN_FLIGHT> post_processDescriptorSets;
}

namespace clz::renderer
{
	/// @brief Create's post process descriptor resources
	/// @return True on succesful creation, false otherwise
	bool createPostProcessDescriptor();

	/// @brief Returns a vector of VkDescriptorPoolSize. Right now total number is 10
	/// @return vector of VkDescriptorPoolSize
	std::vector<VkDescriptorPoolSize> getPostProcessDescriptorPoolSizes();

	/// @brief Allocates post_process descriptor
	/// @param descriptorPool Descriptor pool to carve out memory from
	/// @return True on succesful creation, false otherwise
	bool allocatePostProcessDescriptorSets(const VkDescriptorPool& descriptorPool);

	/// @brief updates post-process descriptor sets
	/// @warning To be called only called after Post process system has been initialized
	void updatePostProcessDescriptorSets();

	/// @brief Hint's post process system that engine's state has changed
	/// mainly used to update the descriptor sets about everything
	void hintPostProcessStateChange();

	/// @brief Destroy's all post process descriptor sets
	void destroyPostProcessDescriptor();
}
