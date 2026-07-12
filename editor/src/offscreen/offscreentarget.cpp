#include "../../include/offscreen/offscreentarget.hpp"
#include "../../include/offscreen/backend/backend.hpp"
#include "core/logs.hpp"
#include "imgui_impl_vulkan.h"
#include "include/inspector/rigidbodycomponent.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/singletimecommand.hpp"
#include "renderer/vk_types.hpp"

namespace clz::editor
{
	bool createOffscreenTarget(OffscreenTarget& target, uint32_t width, uint32_t height);
	void destroyOffscreenTarget(OffscreenTarget& target);
}

namespace clz::editor
{
	bool createOffscreenTargets()
	{
		// Initialize backend
		if (!backend::init())
		{
			clz::log::error("failed to initialize editor's offscreen backend");
			return false;
		}

		// Create offscreen target's

		if (!createOffscreenTarget(physicsBodyShapeImage, 512, 512))
		{
			clz::log::error("Could not create physics body offscreen target for editor");
			return false;
		}
		return true;
	}

	void destroyOffscreenTargets()
	{
		backend::shutdown();
		destroyOffscreenTarget(physicsBodyShapeImage);

		clz::log::info("closed offscreen targets");
	}

	void drawOffscreenTargets(VkCommandBuffer commandBuffer)
	{
		bool isDrawing = false;

		if (physicsBodyShapeImage.showTarget)
		{
			drawBodyEditorOffscreenImage(commandBuffer);
			isDrawing = true;
		}


		IsCurrentlyShowingOffscreenTargets = isDrawing;
	}

	void presentOffscreenWindows()
	{
		bool isPresenting = false;
		if (physicsBodyShapeImage.showTarget)
		{
			presentBodyEditorWindow();
			isPresenting = true;
		}

		IsCurrentlyShowingOffscreenTargets = isPresenting;
	}

	bool isCurrentlyShowingOffscreenTargets()
	{
		return IsCurrentlyShowingOffscreenTargets;
	}

	void flagOffscreenTargetsEntitiesLoaded()
	{
		backend::flagBackendComponentsLoaded();
	}

	bool recreateOffscreenTarget(OffscreenTarget& target, uint32_t width, uint32_t height)
	{
		vkDeviceWaitIdle(renderer::r_deviceContext.device);
		clz::log::info("recreating some offscreen target");
		destroyOffscreenTarget(target);
		if (!createOffscreenTarget(target, width, height))
		{
			clz::log::error("failed to re-create offscreen target");
			return false;
		}


		return true;
	}
} // namespace clz::editor

namespace clz::editor
{
	bool createOffscreenTarget(OffscreenTarget& target, const uint32_t width, const uint32_t height)
	{
		if (width == 0 || height == 0)
		{
			clz::log::warn("body-shape render target extent's are null");
			return true;
		}
		target.extent = VkExtent2D{.width = width, .height = height};

		if (!renderer::createImage(target.image, "editor body-shape", width, height,
			renderer::r_swapchainContext.format.format, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0))
		{
			clz::log::error("failed to create offscreenBodyImage");
			return false;
		}

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(renderer::r_deviceContext.device, target.image, &memReq);
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = renderer::findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (vkAllocateMemory(renderer::r_deviceContext.device, &allocInfo,
			nullptr, &target.imageMemory) != VK_SUCCESS)
		{
			clz::log::error("failed to allocate memory for offscreen shape");
			return false;
		}
		vkBindImageMemory(renderer::r_deviceContext.device, target.image, target.imageMemory, 0);

		if (!renderer::createImageView(target.imageView, "offscreen image view", target.image,
			renderer::r_swapchainContext.format.format, VK_IMAGE_ASPECT_COLOR_BIT))
		{
			clz::log::error("failed to create offscreenImageView");
			return false;
		}

		// Sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(renderer::r_deviceContext.device, &samplerInfo, nullptr, &target.sampler))
		{
			clz::log::error("failed to create sampler");
			return false;
	
		}

		if (renderer::r_swapchainContext.depthFormat == VK_FORMAT_UNDEFINED) [[unlikely]]
		{
			clz::log::error("No suitable depth format was set initially??");
			return false;
		}

		if (!clz::renderer::createImage(target.depthImage, "editor Depth Image", width,
						height, renderer::r_swapchainContext.depthFormat, VK_IMAGE_TILING_OPTIMAL,
						VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0))
		{
			clz::log::error("Could not create depth image for editor");
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(clz::renderer::r_deviceContext.device, target.depthImage, &memRequirements);

		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memRequirements.size;
		memAllocInfo.memoryTypeIndex = renderer::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(clz::renderer::r_deviceContext.device, &memAllocInfo, nullptr, &target.depthImageMemory) !=
		    VK_SUCCESS)
		{
			clz::log::error("vulkan could not create depth memory");
			return false;
		}

		vkBindImageMemory(clz::renderer::r_deviceContext.device, target.depthImage, target.depthImageMemory, 0);

		if (!renderer::createImageView(target.depthImageView, "Depth image view", target.depthImage,
				     renderer::r_swapchainContext.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT))
		{
			clz::log::error("Could not create depth image view");
			return false;
		}

		VkCommandBuffer commandBuffer = renderer::startSingleTimeCommand();

		renderer::transition_image_layout(target.depthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
					VK_PIPELINE_STAGE_2_NONE,
					VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_NONE,
					VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					VK_IMAGE_ASPECT_DEPTH_BIT, commandBuffer);

		renderer::submitSingleTimeCommand(commandBuffer);

		target.descriptorSet = ImGui_ImplVulkan_AddTexture(
			target.sampler,
			target.imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		if (target.descriptorSet != VK_NULL_HANDLE)
		{
			clz::log::debug("created offscreen target in editor");
		}

		return true;
	}

	void destroyOffscreenTarget(OffscreenTarget& target)
	{
		ImGui_ImplVulkan_RemoveTexture(target.descriptorSet);
		vkDestroyImageView(renderer::r_deviceContext.device, target.depthImageView, nullptr);
		vkDestroyImage(renderer::r_deviceContext.device, target.depthImage, nullptr);
		vkFreeMemory(renderer::r_deviceContext.device, target.depthImageMemory, nullptr);
		vkDestroySampler(renderer::r_deviceContext.device, target.sampler, nullptr);
		vkDestroyImageView(renderer::r_deviceContext.device, target.imageView, nullptr);
		vkDestroyImage(renderer::r_deviceContext.device, target.image, nullptr);
		vkFreeMemory(renderer::r_deviceContext.device, target.imageMemory, nullptr);
	}
}
