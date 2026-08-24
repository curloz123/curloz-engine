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
	 * @param baseMipLevel    Base mip-map level. Is 0 by default
	 * @param levelCount	  Number of mip-maps. Is 1 by default
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
		VkCommandBuffer commandBuffer,
		uint32_t baseMipLevel = 0,
		uint32_t levelCount = 1
	);

	/**
	 * @brief Calculates total number of mip-levels of the image
	 * Takes maximum of height and width and decides how many times
	 * can it be divided by 2.
	 * @param width Width of image
	 * @param height Height of image
	 * @note If mip-levels are 5, then 0 is the base image. 1,2,3,4 are the low res mips.
	 * @return Mip levels of image
	 */
	uint32_t calculateMipLevels(uint32_t width, uint32_t height);

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
	 * @param Number of mipmaps to generate. If you are passing a value more than 1, be sure to add TRANSFER_SRC_BIT usage flag to flags parameter.
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
		VkImageCreateFlags flags,
		uint32_t numMipmaps = 1
	);

	/**
	 * @brief Creates an image view for an image
	 *
	 * @param rImageView Reference of image view to create
	 * @param name Name of image view, required for debugging purposes
	 * @param image Image of which this view will view into
	 * @param format Format of the image view
	 * @param aspect What was this again??
	 * @param mipCount Number of mipmaps this image has, by default set to 1
	 * @return True if successful, false if not
	 */
	bool createImageView(
		VkImageView& rImageView,
		const std::string& name,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspect,
		uint32_t mipCount = 1
	);

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
	 * @param maximum level of detail. Is 0 by default
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
		VkSamplerAddressMode addressModeW,
		float maxLod = 0.0f
	);

	/// @brief Sampler filters
	enum class SamplerFilter
	{
		Linear,
		Nearest
	};
	/// @brief mip-map modes
	enum class MipmapMode
	{
		None,
		Linear,
		Nearest
	};
	/// @brief Sampler address modes
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
	 * @param maximum level of detail. Is 0 by default
	 * @return True on creation, false if anything fails
	 */
	bool createSampler(
		VkSampler& rSampler,
		std::string_view name,
		SamplerFilter magFilter,
		SamplerFilter minFilter,
		MipmapMode mipmapMode,
		SamplerAddressMode addressModeU,
		SamplerAddressMode addressModeV,
		SamplerAddressMode addressModeW,
		float maxLod = 0.0f
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


	/**
	 * @brief Finds format you are trying to create the image with is supported or not.
	 * @param candidates All the formats that you can create the image with
	 * @param tiling Tiling of the image. Optimal or linear.
	 * @param features of the format
	 * @return Supported format among those. Else if none is supported, logs an error and returns a string.
	 */
	std::expected<VkFormat, std::string> findSupportedFormat(

		std::span<VkFormat> candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	);

	/**
	 * @brief Generates mip-maps for the images
	 * @param commandBuffer active command buffer to submit this generation command
	 * @param rImage reference to image handle
	 * @param imageWidth width of the image
	 * @param imageHeight height of the image
	 * @param mipLevels number of mip-levels
	 * @note Make sure all the mips of your image are in TRANSFER_DST_OPTIMAL layout before generating mip-maps
	 * @note Also make sure to transition them to your desired layout properly after calling this function
	 */
	void generateMipmaps(VkCommandBuffer commandBuffer, VkImage& rImage, int32_t imageWidth, int32_t imageHeight, uint32_t mipLevels);

	/**
	 * @brief Transitions all mip-levels to a particular image layout
	 * @param rImage reference to image handle
	 * @param finalLayout image's final layout
	 * @param mipLevels number of mip-levels
	 */
	/// @TODO
	void transionAllMipLevels(VkImage& rImage, VkImageLayout finalLayout, uint32_t mipLevels);

} // namespace clz::renderer
