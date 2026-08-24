/**
 * @file image.cpp
 * @author curl0z
 * @brief Vulkan image layout transition utility implementation.
 */

#include "renderer/utility/image.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/utility/namer.hpp"
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>
#include <cmath>
#include "core/assert.hpp""

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
		VkCommandBuffer commandBuffer,
		const uint32_t baseMipLevel,
		const uint32_t levelCount
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
			.baseMipLevel = baseMipLevel,
			.levelCount = levelCount,
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

	/// @copydoc calculateMipLevels
	uint32_t calculateMipLevels(const uint32_t width, const uint32_t height)
	{
		return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
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
		const VkImageCreateFlags flags,
		const uint32_t numMipmaps
	)
	{

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = numMipmaps;
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

		setHandleName(
			reinterpret_cast<uint64_t>(rImage),
			VK_OBJECT_TYPE_IMAGE,
			name.c_str()
		);

		return true;
	}

	/// @copydoc createImageView
	bool createImageView(
		VkImageView& rImageView,
		const std::string& name,
		const VkImage image,
		const VkFormat format,
		const VkImageAspectFlags aspect,
		const uint32_t mipCount
	)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspect;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipCount;
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

		setHandleName(
			reinterpret_cast<uint64_t>(rImageView),
			VK_OBJECT_TYPE_IMAGE_VIEW,
			name.c_str()
		);

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
		const VkSamplerAddressMode addressModeW,
		const float maxLod
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
		samplerInfo.maxLod = maxLod;

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
	bool createSampler(
		VkSampler& rSampler,
		const std::string_view name,
		const SamplerFilter magFilter,
		const SamplerFilter minFilter,
		const MipmapMode mipmapMode,
		const SamplerAddressMode addressModeU,
		const SamplerAddressMode addressModeV,
		const SamplerAddressMode addressModeW,
		const float maxLod
	)
	{
		const auto toVkFilter = [](const SamplerFilter filter) -> VkFilter
		{
			switch (filter)
			{
				case SamplerFilter::Linear:
					return VK_FILTER_LINEAR;

				case SamplerFilter::Nearest:
					return VK_FILTER_NEAREST;
			}

			CLZ_ASSERT(false, "Invalid sampler filter");
			return VK_FILTER_LINEAR;
		};

		const auto toVkMipmapMode = [](const MipmapMode mode) -> VkSamplerMipmapMode
		{
			switch (mode)
			{
				case MipmapMode::None:
				case MipmapMode::Nearest:
					return VK_SAMPLER_MIPMAP_MODE_NEAREST;

				case MipmapMode::Linear:
					return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			}

			CLZ_ASSERT(false, "Invalid mipmap mode");
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		};

		const auto toVkAddressMode = [](const SamplerAddressMode mode) -> VkSamplerAddressMode
		{
			switch (mode)
			{
				case SamplerAddressMode::CLAMP_TO_EDGE:
					return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

				case SamplerAddressMode::CLAMP_TO_BORDER:
					return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

				case SamplerAddressMode::MIRRORED_REPEAT:
					return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

				case SamplerAddressMode::REPEAT:
					return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}

			CLZ_ASSERT(false, "Invalid sampler address mode");
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		};

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		samplerInfo.magFilter = toVkFilter(magFilter);
		samplerInfo.minFilter = toVkFilter(minFilter);

		samplerInfo.mipmapMode = toVkMipmapMode(mipmapMode);

		samplerInfo.addressModeU = toVkAddressMode(addressModeU);
		samplerInfo.addressModeV = toVkAddressMode(addressModeV);
		samplerInfo.addressModeW = toVkAddressMode(addressModeW);

		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = maxLod;

		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(
			    r_deviceContext.device,
			    &samplerInfo,
			    nullptr,
			    &rSampler
		    ) != VK_SUCCESS)
		{
			clz::log::error(
				"failed to create " +
				std::string(name) +
				" sampler"
			);
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
				{0, 0, 0},
				{static_cast<int32_t>(srcExtent.width), static_cast<int32_t>(srcExtent.height), 1}
			},
			.dstSubresource = subresource,
			.dstOffsets = {
				{0, 0, 0},
				{static_cast<int32_t>(dstExtent.width), static_cast<int32_t>(dstExtent.height), 1}
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

	/// @copydoc generateMipmaps
	void generateMipmaps(VkCommandBuffer commandBuffer, VkImage& rImage, const int32_t imageWidth, const int32_t imageHeight, const uint32_t mipLevels)
	{
		auto mipWidth = imageWidth;
		auto mipHeight = imageHeight;

		for (uint32_t i = 1; i < mipLevels; ++i)
		{
			transition_image_layout(
				rImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_ACCESS_2_TRANSFER_WRITE_BIT,
				VK_ACCESS_2_TRANSFER_READ_BIT,
				VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer,
				i-1,
				1
			);
			VkImageBlit blit = {};

			blit.srcOffsets[0] = {0, 0, 0};
			blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[0] = {0, 0, 0};
			blit.dstOffsets[1] = {
				(mipWidth > 1) ? mipWidth / 2 : 1, 
				(mipHeight > 1) ? mipHeight / 2 : 1, 
				1
			};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(
				commandBuffer,
				rImage,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				rImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, 
				&blit,
				VK_FILTER_LINEAR
			);

			mipWidth  /= 2;
			mipHeight /= 2;
		}
		
	}
} // namespace clz::renderer
