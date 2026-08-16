#pragma once

#include "renderer/vk_types.hpp"
#include "renderer/utility/descriptor.hpp"
#include <array>
#include <vector>

namespace clz::renderer
{
	inline constexpr uint8_t POST_PROCESS_SET_POINT = 3;
	inline constexpr uint8_t PRE_TONEMAP_IMAGE_BIND_POINT = 0;
	inline constexpr uint8_t BLOOM_IMAGE_BIND_POINT = 1;
	inline constexpr uint8_t TONEMAP_IMAGE_BIND_POINT = 2;
	inline constexpr uint8_t POST_TONEMAP_IMAGE_BIND_POINT = 3;
	inline VkDescriptorSetLayout post_processDescriptorLayout;
	inline std::array<VkDescriptorSet, r_FRAMES_IN_FLIGHT> post_processDescriptorSets;
}

namespace clz::renderer
{
	bool createPostProcessDescriptor();
	std::vector<VkDescriptorPoolSize> getPostProcessDescriptorPoolSizes();
	bool allocatePostProcessDescriptorSets(const VkDescriptorPool& descriptorPool);

	/// @brief updates post-process descriptor sets
	/// @warning To be called only called after editor has been initialized
	/// or after post process system has initialized in non editor build
	/// and not by the general descriptor update function
	void updatePostProcessDescriptorSets();

	void hintPostProcessStateChange();
	void destroyPostProcessDescriptor();
}