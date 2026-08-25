/**
 * @file texture.hpp
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
	/// @brief texture's set point
	inline constexpr uint8_t TEXTURE_SET_POINT = 1;
	/// @brief texture bind point
	inline constexpr uint8_t TEXTURE_BIND_POINT = 0;
	/// @brief Texture descriptor set layout
	inline VkDescriptorSetLayout textureDescriptorLayout;

} // namespace clz::renderer

namespace clz::renderer
{
	/// @brief Create's texture descriptor resources
	/// Rn only creates the layout
	/// @return True on success
	bool createTextureDescriptor();

	/// @brief Writes back all entity's texture data to texture descriptor sets.
	/// @note Only called after entities have been initializes
	void updateTextureForDescriptors(const std::vector<VkDescriptorSet>& descriptorSets);

	/// @brief Destroys texture descriptor
	void destroyTextureDescriptor();

} // namespace clz::renderer
