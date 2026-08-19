/**
 * @file descriptor.cpp
 * @author curl0z
 * @brief Implementation of the descriptor set layout helper (see descriptor.hpp).
 */

#include "renderer/utility/descriptor.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{

	/// @copydoc createDescriptorLayout
	bool createDescriptorLayout(
		VkDescriptorSetLayout& rLayout,
		const std::span<const uint32_t> rBindPoints,
		const std::span<const VkDescriptorType> rDescriptorTypes,
		const std::span<const uint32_t> rDescriptorCounts,
		const std::span<const VkShaderStageFlags> rShaderStages,
		const std::string& layoutName,
		const bool isTextureLayout
	)

	{
		// All parallel arrays must describe the same set of bindings.
		CLZ_ASSERT(
			rBindPoints.size() == rDescriptorTypes.size(),
			"while sending: " + layoutName + "size of bindings and types is not equal"
		);
		CLZ_ASSERT(
			rDescriptorTypes.size() == rDescriptorCounts.size(),
			"while sending: " + layoutName + "size of counts and types is not equal"
		);
		CLZ_ASSERT(
			rDescriptorCounts.size() == rShaderStages.size(),
			"while sending: " + layoutName +
				"size of counts and shader stages is not equal"
		);

		// Build one VkDescriptorSetLayoutBinding per entry.
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		for (size_t i = 0; i < rBindPoints.size(); ++i)
		{
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = rBindPoints[i];
			layoutBinding.descriptorType = rDescriptorTypes[i];
			layoutBinding.descriptorCount = rDescriptorCounts[i];
			layoutBinding.stageFlags = rShaderStages[i];

			layoutBindings.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		// Bindless-texture-array support: lets slots be written after the
		// descriptor set is bound (UPDATE_AFTER_BIND) and lets unused slots
		// stay unwritten (PARTIALLY_BOUND) — needed since textures are
		// registered incrementally as models load, not all at once.
		const std::vector<VkDescriptorBindingFlags> bindingFlags(
			rBindPoints.size(),
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
				VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		);
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = bindingFlags.size();
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();

		if (isTextureLayout)
		{
			layoutInfo.flags =
				VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
			layoutInfo.pNext = &bindingFlagsInfo;
		}
		else
		{
			layoutInfo.pNext = nullptr;
			layoutInfo.flags = 0;
		}

		if (vkCreateDescriptorSetLayout(
			    r_deviceContext.device,
			    &layoutInfo,
			    nullptr,
			    &rLayout
		    ) != VK_SUCCESS)
		{
			clz::log::error("Could not create descriptor layout: " + layoutName);
			return false;
		}

		setHandleName(
			reinterpret_cast<uint64_t>(rLayout),
			VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
			layoutName.c_str()
		);
		return true;
	}

	/// @copydoc destroyDescriptorSetLayout
	void destroyDescriptorSetLayout(VkDescriptorSetLayout& rLayout)
	{
		vkDestroyDescriptorSetLayout(r_deviceContext.device, rLayout, nullptr);
	}
} // namespace clz::renderer
