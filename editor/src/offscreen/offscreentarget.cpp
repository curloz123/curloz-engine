/**
 * @file offscreentarget.cpp
 * @author curl0z
 * @brief Lifecycle management for editor offscreen render targets:
 * creation/destruction of the color+depth image pair, sampler, and ImGui
 * texture registration, plus per-frame dispatch of draw/present calls for
 * every currently-open offscreen window.
 */

#include "../../include/offscreen/offscreentarget.hpp"
#include "core/logs.hpp"
#include "imgui_impl_vulkan.h"
#include "renderer/arena.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/utility/image.hpp"
#include "../../include/inspector/rigidbodycomponent.hpp"

namespace clz::editor
{
	void drawOffscreenImages(VkCommandBuffer commandBuffer)
	{
		if (bodyEditorImage.isShowingImage())
		{
			showBodyEditorWindow(commandBuffer);
		}
	}

	void OffscreenImage::initialize(
		const uint32_t width,
		const uint32_t height
	)
	{
		m_showImage = false;

		m_outDated = false;

		renderer::CameraDef bodyCameraDef = {};
		bodyCameraDef.position = math::vec3(0.0f, 0.0f, 1.0f);
		bodyCameraDef.localFront = math::vec3(0.0f, 0.0f, -1.0f);
		bodyCameraDef.yaw = -90.0f;
		bodyCameraDef.pitch = 0.0f;
		bodyCameraDef.acceleration = 10000.0f;
		m_cameraId = renderer::createCamera(bodyCameraDef);

		m_extent = {width, height};
	}


	void OffscreenImage::destroy()
	{
		const auto& device = renderer::r_deviceContext.device;

		if (m_imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(
				device,
				m_imageView,
				nullptr
			);
			m_imageView = VK_NULL_HANDLE;
		}
		if (m_depthImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(
				device,
				m_depthImageView,
				nullptr
			);
			m_depthImageView = VK_NULL_HANDLE;
		}

		if (m_image != VK_NULL_HANDLE)
		{
			vkDestroyImage(
				device,
				m_image,
				nullptr
			);
			m_image = VK_NULL_HANDLE;
		}
		if (m_depthImage != VK_NULL_HANDLE)
		{
			vkDestroyImage(
				device,
				m_depthImage,
				nullptr
			);
			m_depthImage = VK_NULL_HANDLE;
		}

		if (!m_imageMemory.isNull())
		{
			renderer::freeImageMemory(m_imageMemory);
		}
		if (!m_depthImageMemory.isNull())
		{
			renderer::freeImageMemory(m_depthImageMemory);
		}

		if (m_descriptorSet != VK_NULL_HANDLE)
		{
			ImGui_ImplVulkan_RemoveTexture(m_descriptorSet);
			m_descriptorSet = VK_NULL_HANDLE;
		}
	}

	bool OffscreenImage::prepare(VkCommandBuffer commandBuffer)
	{
		if (m_showImage && !m_outDated)
			return true;

		/// If image is outdated, destroy all current resources first
		if (m_outDated)
		{
			freeFrameResources();
			m_outDated = false;
		}

		/// Now simply allocate resources
		if (!renderer::createImage(
			    m_image,
			    "offscreen image",
			    m_extent.width,
			    m_extent.height,
			    renderer::r_renderTargetContext.imageFormat,
			    VK_IMAGE_TILING_OPTIMAL,
			    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			    0
		))
		{
			clz::log::error("failed to create offscreen image");
			return false;
		}
		if (!clz::renderer::createImage(
			m_depthImage,
			"offscreen depth Image",
			m_extent.width,
			m_extent.height,
			renderer::r_renderTargetContext.depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			0
		))
		{
			clz::log::error("Could not create offscreen image for editor");
			return false;
		}

		VkMemoryRequirements memReq;
		VkMemoryRequirements depthMemReq;
		vkGetImageMemoryRequirements(
			renderer::r_deviceContext.device,
			m_image,
			&memReq
		);
		vkGetImageMemoryRequirements(
			renderer::r_deviceContext.device,
			m_depthImage,
			&depthMemReq
		);

		const renderer::ImageMemoryAllocInfo allocInfo{
			.imageType = renderer::ImageType::CANVAS,
			.alignment = memReq.alignment,
			.size = memReq.size
		};
		const renderer::ImageMemoryAllocInfo dAllocInfo{
			.imageType = renderer::ImageType::DEPTH,
			.alignment = depthMemReq.alignment,
			.size = depthMemReq.size
		};
		if (!renderer::carveImageMemory(allocInfo, m_imageMemory))
		{
			clz::log::error("Could not carve offscreen image memory");
			return false;
		}
		if (!renderer::carveImageMemory(dAllocInfo, m_depthImageMemory))
		{
			clz::log::error("Could not carve offscreen depth image memory");
			return false;
		}
		renderer::bindImageMemory(m_image, m_imageMemory);
		renderer::bindImageMemory(m_depthImage, m_depthImageMemory);

		if (!renderer::createSampler(
			m_imageSampler,
			"offscreen image sampler",
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
		))
		{
			clz::log::error("Could not create offscreen image sampler");
			return false;
		}

		m_descriptorSet = ImGui_ImplVulkan_AddTexture(
			m_imageSampler,
			m_imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		if (m_descriptorSet == VK_NULL_HANDLE)
		{
			clz::log::error(
				"Imgui could not create offscreen image's descriptor"
			);
			return false;
		}

		renderer::transition_image_layout(
			m_depthImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_NONE,
			VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
				VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_NONE,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			commandBuffer
		);

		m_showImage = true;

		return true;
	}

	/// Frees the image resources,
	/// always call this after use has been completed
	void OffscreenImage::freeFrameResources()
	{
		const auto& device = renderer::r_deviceContext.device;

		if (m_imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(
				device,
				m_imageView,
				nullptr
			);
			m_imageView = VK_NULL_HANDLE;
		}
		if (m_depthImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(
				device,
				m_depthImageView,
				nullptr
			);
			m_depthImageView = VK_NULL_HANDLE;
		}

		if (m_image != VK_NULL_HANDLE)
		{
			vkDestroyImage(
				device,
				m_image,
				nullptr
			);
			m_image = VK_NULL_HANDLE;
		}
		if (m_depthImage != VK_NULL_HANDLE)
		{
			vkDestroyImage(
				device,
				m_depthImage,
				nullptr
			);
			m_depthImage = VK_NULL_HANDLE;
		}

		if (!m_imageMemory.isNull())
		{
			renderer::freeImageMemory(m_imageMemory);
			m_imageMemory.makeNull();
		}
		if (!m_depthImageMemory.isNull())
		{
			renderer::freeImageMemory(m_depthImageMemory);
			m_depthImageMemory.makeNull();
		}

		if (m_descriptorSet != VK_NULL_HANDLE)
		{
			ImGui_ImplVulkan_RemoveTexture(m_descriptorSet);
			m_descriptorSet = VK_NULL_HANDLE;
		}

		m_showImage = false;
	}
	void OffscreenImage::setExtent(const uint32_t width, const uint32_t height)
	{
		if (m_extent.width != width || m_extent.height != height)
		{
			m_outDated = true;
			m_extent = {width, height};
		}
	}
}
