/**
 * @file image.cpp
 * @author curl0z
 * @brief Vulkan image layout transition utility implementation.
 */

#include "renderer/utility/image.hpp"
#include "core/logs.hpp"
#include "nlohmann/adl_serializer.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/utility/namer.hpp"

#include <string>

namespace clz::renderer
{
	/// @copydoc transition_image_layout
	void transition_image_layout(
		const VkImage image,
		const VkImageLayout oldLayout,
		const VkImageLayout newLayout,
		const VkAccessFlags2 src_access_mask,
		const VkAccessFlags2 dst_access_mask,
		const VkPipelineStageFlags2 src_stage_mask,
		const VkPipelineStageFlags2 dst_stage_mask,
		VkImageAspectFlags aspectMask,
		VkCommandBuffer commandBuffer
	)
	{
		VkImageMemoryBarrier2 barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.srcStageMask = src_stage_mask;
		barrier.dstStageMask = dst_stage_mask;
		barrier.srcAccessMask = src_access_mask;
		barrier.dstAccessMask = dst_access_mask;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange = {
			.aspectMask = aspectMask,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		VkDependencyInfo dependencyInfo = {};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pImageMemoryBarriers = &barrier;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.dependencyFlags = 0;
		dependencyInfo.pNext = nullptr;

		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
	}

	/// @copydoc @createImage
	bool createImage(
		VkImage& rImage,
		const std::string& name,
		const uint32_t width,
		const uint32_t height,
		const VkFormat format,
		const VkImageTiling tiling,
		const VkImageUsageFlags usage,
		const VkImageCreateFlags flags
	)
	{

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = flags;

		if (vkCreateImage(
			    clz::renderer::r_deviceContext.device,
			    &imageInfo,
			    nullptr,
			    &rImage
		    ) != VK_SUCCESS)
		{
			clz::log::error(
				"vulkan Unable to create "
				"image handle for" +
				name
			);
			return false;
		}

		return true;
	}

	/// @copydoc createImageView
	bool createImageView(
		VkImageView& rImageView,
		const std::string& name,
		const VkImage image,
		const VkFormat format,
		const VkImageAspectFlags aspect
	)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspect;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(
			    clz::renderer::r_deviceContext.device,
			    &viewInfo,
			    nullptr,
			    &rImageView
		    ) != VK_SUCCESS)
		{
			clz::log::error(
				"vulkan Unable to create "
				"image view handle for" +
				name
			);
			return false;
		}

		return true;
	}

	/// @copydoc createSampler
	bool createSampler(
		VkSampler& rSampler,
		const std::string_view name,
		const VkFilter magFilter,
		const VkFilter minFilter,
		const VkSamplerMipmapMode mipmapMode,
		const VkSamplerAddressMode addressModeU,
		const VkSamplerAddressMode addressModeV,
		const VkSamplerAddressMode addressModeW
	)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.addressModeU = addressModeU;
		samplerInfo.addressModeV = addressModeV;
		samplerInfo.addressModeW = addressModeW;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(
			    r_deviceContext.device,
			    &samplerInfo,
			    nullptr,
			    &rSampler
		    ) != VK_SUCCESS)
		{
			clz::log::error("failed to create" + std::string(name) + " sampler");
			return false;
		}
		setHandleName(
			reinterpret_cast<uint64_t>(rSampler),
			VK_OBJECT_TYPE_SAMPLER,
			std::string(name).c_str()
		);

		return true;
	}


	/// @copydoc copyImage2D
	void copyImage2D(
		VkImage srcImage,
		const VkExtent2D srcExtent,
		VkImage dstImage,
		const VkExtent2D dstExtent,
		VkCommandBuffer commandBuffer
	)
	{
		const VkImageSubresourceLayers subresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
		};

		const VkImageBlit blitRegion = {
			.srcSubresource = subresource,
			.srcOffsets = {
				{
					0, 0, 0
				},
				{
					static_cast<int32_t>(srcExtent.width),
					static_cast<int32_t>(srcExtent.height),
					1
				}
			},
			.dstSubresource = subresource,
			.dstOffsets = {
				{
					0, 0, 0
				},
				{
					static_cast<int32_t>(dstExtent.width),
					static_cast<int32_t>(dstExtent.height),
					1
				}
			}
		};

		vkCmdBlitImage(
			commandBuffer,
			srcImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&blitRegion,
			VK_FILTER_LINEAR);
	}

	/// @copydoc findSupportedFormat
	std::expected<VkFormat, std::string> findSupportedFormat(
		const std::span<VkFormat> candidates,
		const VkImageTiling tiling,
		const VkFormatFeatureFlags features
	)
	{
		for (auto& format : candidates)
		{
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(
				r_deviceContext.physicalDevice,
				format,
				&formatProperties
			);

			switch (tiling)
			{
			case VK_IMAGE_TILING_OPTIMAL:
				if ((formatProperties.optimalTilingFeatures & features) == features)
				{
					clz::log::info("Optimal depth format found");
					return format;
				}
				break;

			case VK_IMAGE_TILING_LINEAR:
				if ((formatProperties.linearTilingFeatures & features) == features)
				{
					clz::log::info("linear depth format found");
					return format;
				}
				break;

			default:
				clz::log::error("A requested depth format is not available");
			}
		}
		clz::log::error("Could not find any supported depth format");
		return VK_FORMAT_UNDEFINED;
	}


} // namespace clz::renderer
