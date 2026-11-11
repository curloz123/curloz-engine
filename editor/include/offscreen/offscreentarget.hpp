/**
 * @file offscreentarget.hpp
 * @author curl0z
 * @brief This file handles all offscreen images that the editor might require
 */
#pragma once

#include "renderer/camera/camera.hpp"
#include "renderer/arena.hpp"
#include "renderer/camera/cameradata.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace clz::editor
{
	class OffscreenImage
	{
	private:
		/// Flag that tells whether to show image this frame.
		/// Also tells whether image has 'just' started
		/// to being used to draw on, so we could allocate
		/// resources appropriately.
		bool m_showImage = false;

		/// Flag that denotes whether image is outdated or not.
		bool m_outDated = true;

		/// Offscreen camera
		renderer::CameraId m_cameraId = renderer::NULL_CAMERA;

		/// Image extents
		VkExtent2D m_extent = {256, 256};

		/// Image data
		VkImage m_image = VK_NULL_HANDLE;
		/// Image view
		VkImageView m_imageView = VK_NULL_HANDLE;
		/// Image memory
		renderer::clzImageMemory m_imageMemory{};
		/// Image sampler
		VkSampler m_imageSampler = VK_NULL_HANDLE;
		/// Depth image
		VkImage m_depthImage = VK_NULL_HANDLE;
		/// Depth image view
		VkImageView m_depthImageView = VK_NULL_HANDLE;
		/// Depth image memory
		renderer::clzImageMemory m_depthImageMemory{};

		/// Imgui's descriptor set
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

	public:
		OffscreenImage() = default;

		/// Initializes the offscreen image
		void initialize(
			uint32_t width,
			uint32_t height
		);

		/// offscreen image's camera might be initialized later,
		/// much later after its actual initialization, so this func
		/// basically allows changing attributes later.
		void changeCameraAttrib(const renderer::CameraDef& cameraDef);

		/// destroys the offscreen image
		/// Must only be called during engine closure
		void destroy();

		/// @brief Prepares the image to be presented.
		/// After showing has been finished, free its resources
		/// by calling freeResources
		/// @param commandBuffer Also records depth-image transitions
		/// and all those things.
		bool prepare(VkCommandBuffer commandBuffer);

		/// Sets extents of the image,
		/// Next time prepare func is called, it would be resized
		void setExtent(uint32_t width, uint32_t height);

		/// Frees the image's frame resources,
		/// always call this after use has been completed
		void freeFrameResources();

		VkExtent2D getExtent() const
		{
			return m_extent;
		}
		VkImage getVkImage() const
		{
			return m_image;
		}
		VkImageView getVkImageView() const
		{
			return m_imageView;
		}
		VkImage getVkDepthImage() const
		{
			return m_depthImage;
		}
		VkImageView getVkDepthImageView() const
		{
			return m_depthImageView;
		}
		VkSampler getVkSampler() const
		{
			return m_imageSampler;
		}
		VkDescriptorSet getVkDescriptorSet() const
		{
			return m_descriptorSet;
		}
		renderer::CameraId getCameraId() const
		{
			return m_cameraId;
		}
		[[nodiscard]] bool isShowingImage() const
		{
			return m_showImage;
		}
		[[nodiscard]] bool isOutDated() const
		{
			return m_outDated;
		}
	};


	/// This is the single function that internally calls drawing
	/// or update/draw function of all offscreen images.
	/// Must be called each frame.
	/// Don't consider it to be simply a function that draws, you can
	/// technically do anything, as long as its related to offscreen images,
	/// basically involves code that involves drawing on offscreen images.
	void drawOffscreenImages(VkCommandBuffer commandBuffer);

} // namespace clz::editor
