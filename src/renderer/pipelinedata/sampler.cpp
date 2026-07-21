/**
 * @file sampler.cpp
 * @author curl0z
 * @brief Create pipeline's combined sampler storage
 * that will hold all textures in the gpu.
 *
 * @note This may sound similar to textures, matter of fact it is,
 * but since texture suits more with entity data, most of the
 * implementation stays there. This file only contains
 * The descriptor set layouts and bind points.
 * @note update samplers data for your
 * targeted descriptor after scene have been loaded.
 */

#include "renderer/pipelinedata/sampler.hpp"
#include "core/logs.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	/// @brief Creates combined image samplers
	/// @return true on success, else ukw
	bool createCombinedSamplersLayout()
	{
		// Descriptor type - 1
		VkDescriptorSetLayoutBinding textureLayoutBinding = {};
		textureLayoutBinding.binding = COMBINED_SAMPLER_BINDPOINT;
		textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureLayoutBinding.descriptorCount = r_MAX_TEXTURE_COUNT;
		textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		constexpr std::array<VkDescriptorBindingFlags, 1> bindingFlags = {VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
										  VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = bindingFlags.size();
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = &bindingFlagsInfo;
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &textureLayoutBinding;

		if (vkCreateDescriptorSetLayout(clz::renderer::r_deviceContext.device, &layoutInfo, nullptr, &combinedSamplerLayout) != VK_SUCCESS)
		{
			clz::log::error("vulkan Could not initialize descriptor set layout");
			return false;
		}
		setHandleName(reinterpret_cast<uint64_t>(combinedSamplerLayout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
			      "main combined sampler layout");
		return true;
	}

	/// @brief This function is the one that actually writes
	/// data back to the gpu.
	/// @param descriptorSets array of "already initialized" descriptor sets
	/// make sure the size is FRAMES_IN_FLIGHT
	/// @warning MAKE SURE TO CALL IT, and call it after scene have been loaded
	/// precisely in updatePipelineData
	void updateSamplersDataForDescriptorSets(const std::vector<VkDescriptorSet>& descriptorSets)
	{
		if (r_numRegisteredTextures == 0)
		{
			clz::log::warn("No textures registered, "
				       "not updating entity data in descriptor");
			return;
		}
		for (uint32_t i = 0; i < r_numRegisteredTextures; ++i)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = r_textures.imageView[i];
			imageInfo.sampler = r_sampler;

			for (auto j = 0; j < r_FRAMES_IN_FLIGHT; ++j)
			{
				VkWriteDescriptorSet write{};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = descriptorSets[j];
				write.dstBinding = COMBINED_SAMPLER_BINDPOINT;
				write.dstArrayElement = i;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = 1;
				write.pImageInfo = &imageInfo;
				vkUpdateDescriptorSets(r_deviceContext.device, 1, &write, 0, nullptr);
			}
		}
		clz::log::info("initialized entities texture descriptors");
	}

	/// @brief Destroys combined image samplers
	void destroyCombinedImageSamplersLayout()
	{
		vkDestroyDescriptorSetLayout(clz::renderer::r_deviceContext.device, combinedSamplerLayout, nullptr);
	}
} // namespace clz::renderer