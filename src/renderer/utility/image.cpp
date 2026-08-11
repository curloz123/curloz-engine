/**
 * @file image.cpp
 * @author curl0z
 * @brief Vulkan image layout transition utility implementation.
 */

#include "renderer/utility/image.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
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

} // namespace clz::renderer
