/**
* @file descriptor.hpp
 * @author curl0z
 * @brief Generic descriptor set layout creation helper.
 *
 * A single call here builds a VkDescriptorSetLayout from parallel
 * arrays describing each binding — used for camera UBOs, light UBOs/
 * SSBOs, and the bindless texture array layout.
 */

#pragma once

#include <span>
#include <string>
#include <vulkan/vulkan.h>

namespace clz::renderer
{

	/**
	 * @brief Creates a descriptor set layout from parallel binding arrays.
	 *
	 * All four span parameters must be the same length — index i across
	 * all of them describes one binding.
	 *
	 * @param rLayout Output — receives the created layout handle.
	 * @param rBindPoints Binding index for each descriptor.
	 * @param rDescriptorTypes Vulkan descriptor type for each binding
	 * (e.g. VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).
	 * @param rDescriptorCounts Array size (descriptorCount) for each binding.
	 * @param rShaderStages Shader stages each binding is visible to.
	 * @param layoutName Debug name applied to the created layout.
	 * @param isTextureLayout When true, marks the layout with
	 * VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT plus
	 * partially-bound/update-after-bind binding flags — required for the
	 * bindless texture array (set 1) so textures can be registered
	 * incrementally without every slot being written up front.
	 * @return true on success, false if layout creation failed.
	 */
	bool createDescriptorLayout(
		VkDescriptorSetLayout& rLayout,
		std::span<const uint32_t> rBindPoints,
		std::span<const VkDescriptorType> rDescriptorTypes,
		std::span<const uint32_t> rDescriptorCounts,
		std::span<const VkShaderStageFlags> rShaderStages,
		const std::string& layoutName,
		bool isTextureLayout = false
	);

	/**
	 * @brief Destroys a previously created descriptor set layout.
	 * @param rLayout Layout to destroy.
	 */
	void destroyDescriptorSetLayout(VkDescriptorSetLayout& rLayout);
} // namespace clz::renderer