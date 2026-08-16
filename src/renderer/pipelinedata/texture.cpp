/**
 * @file texture.cpp
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

#include "renderer/pipelinedata/texture.hpp"
#include "core/logs.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/utility/descriptor.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	bool createTextureDescriptor()
	{

		const std::vector<uint32_t> bindPoints = {TEXTURE_BIND_POINT};
		const std::vector<VkDescriptorType> descriptorTypes = {
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		const std::vector<uint32_t> descriptorCounts = {r_MAX_TEXTURE_COUNT};
		const std::vector<VkShaderStageFlags> shaderStages = {VK_SHADER_STAGE_FRAGMENT_BIT};
		if (!createDescriptorLayout(
			    textureDescriptorLayout,
			    bindPoints,
			    descriptorTypes,
			    descriptorCounts,
			    shaderStages,
			    "texture descriptor layout",
			    true
		    ))
		{
			clz::log::error("unable to create textures descriptor layout");
			return false;
		}

		return true;
	}

	/// @brief This function is the one that actually writes
	/// data back to the gpu.
	/// @param descriptorSets array of "already initialized" descriptor sets
	/// make sure the size is FRAMES_IN_FLIGHT
	/// @warning MAKE SURE TO CALL IT, and call it after scene have been loaded
	/// precisely in updatePipelineData
	void updateTextureForDescriptors(const std::vector<VkDescriptorSet>& descriptorSets)
	{
		if (r_numRegisteredTextures == 0)
		{
			clz::log::warn(
				"No textures registered, "
				"not updating entity data in descriptor"
			);
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
				write.dstBinding = TEXTURE_BIND_POINT;
				write.dstArrayElement = i;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = 1;
				write.pImageInfo = &imageInfo;
				vkUpdateDescriptorSets(
					r_deviceContext.device,
					1,
					&write,
					0,
					nullptr
				);
			}
		}
		clz::log::info("initialized entities texture descriptors");
	}

	void destroyTextureDescriptor()
	{
		destroyDescriptorSetLayout(textureDescriptorLayout);
	}

} // namespace clz::renderer