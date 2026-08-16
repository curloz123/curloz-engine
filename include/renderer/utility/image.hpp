/**
 * @file image.hpp
 * @authos curl0z
 * @brief Contains all the functions concerning images
 */
#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <expected>
#include <span>

namespace clz::renderer
{
	/**
	 * @brief Inserts a pipeline barrier to transition an image's layout.
	 * @param image           Image to transition.
	 * @param oldLayout       Current image layout.
	 * @param newLayout       Target image layout.
	 * @param src_access_mask Access mask for the source stage (what was written).
	 * @param dst_access_mask Access mask for the destination stage (what will
	 * read/write).
	 * @param src_stage_mask  Pipeline stage that produced the data.
	 * @param dst_stage_mask  Pipeline stage that will consume the data.
	 * @param commandBuffer   Command buffer to record the barrier into.
	 * @param aspectMask	  Subresource range aspect mark.
	 */
	void transition_image_layout(
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkAccessFlags2 src_access_mask,
		VkAccessFlags2 dst_access_mask,
		VkPipelineStageFlags2 src_stage_mask,
		VkPipelineStageFlags2 dst_stage_mask,
		VkImageAspectFlags aspectMask,
		VkCommandBuffer commandBuffer
	);

	/**
	 * @brief Creates an image
	 *
	 * @param rImage Reference of image to create
	 * @param name Name of the image, required for debugging purposes
	 * @param width Extent.width of image
	 * @param height Extent.height of image
	 * @param format Format of the image
	 * @param tiling Optimal or linear??
	 * @param usage Usage flags
	 * @param flags which flags are these again??
	 * @return True if successful, false if not
	 */
	bool createImage(
		VkImage& rImage,
		const std::string& name,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkImageCreateFlags flags
	);

	/**
	 * @brief Creates an image view for an image
	 *
	 * @param rImageView Reference of image view to create
	 * @param name Name of image view, required for debugging purposes
	 * @param image Image of which this view will view into
	 * @param format Format of the image view
	 * @param aspect What was this again??
	 * @return True if successful, false if not
	 */
	bool createImageView(
		VkImageView& rImageView,
		const std::string& name,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspect
	);

	enum class SamplerFilter
	{
		LINEAR,
		NEAREST
	};
	enum class SamplerAddressMode
	{
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER,
		MIRRORED_REPEAT,
		REPEAT
	};
	/**
	 *
	 * @param rSampler Sampler to create
	 * @param name Name of sampler
	 * @param magFilter Magnification filter
	 * @param minFilter Minimization filter
	 * @param mipmapMode Mim-map mode of sampler
	 * @param addressModeU Address Mode U
	 * @param addressModeV Address Mode V
	 * @param addressModeW Address Mode W
	 * @return True on creation, false if anything fails
	 */
	bool createSampler(
		VkSampler& rSampler,
		std::string_view name,
		VkFilter magFilter,
		VkFilter minFilter,
		VkSamplerMipmapMode mipmapMode,
		VkSamplerAddressMode addressModeU,
		VkSamplerAddressMode addressModeV,
		VkSamplerAddressMode addressModeW
	);


	/**
	 * @brief Copies one image to another
	 *
	 * @param srcImage Source image (the one to copy)
	 * @param srcExtent Source image extents
	 * @param dstImage Destination image (the one to be copied one)
	 * @param dstExtent Destination image extents
	 * @param commandBuffer Active command buffer
	 */
	void copyImage2D(
		VkImage srcImage,
		VkExtent2D srcExtent,
		VkImage dstImage,
		VkExtent2D dstExtent,
		VkCommandBuffer commandBuffer
	);


	std::expected<VkFormat, std::string> findSupportedFormat(
		std::span<VkFormat> candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	);
} // namespace clz::renderer