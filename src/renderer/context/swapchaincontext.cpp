/**
 * @file swapchaincontext.cpp
 * @author curl0z
 * @brief Implementation of the initialization
 * and cleanup of swapchain context
 */

#include "renderer/context/swapchaincontext.hpp"
#include "config/config.hpp"
#include "core/logs.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/singletimecommand.hpp"
#include "renderer/vk_types.hpp"
#include "window/window.hpp"
#include <limits>
#include <string>

namespace clz::renderer
{
bool initSwapchainContext()
{
	if (!createSwapchain())
	{
		clz::log::error("Failed to create swapchain");
		clz::log::error("Could not initialize swapchain context");
		return false;
	}

	if (!createDepthResources())
	{
		clz::log::error("Failed to create depth resources");
		clz::log::error("Could not initialize swapchain context");
		return false;
	}

	clz::log::info("Initialized swapchain context");
	return true;
}

bool createSwapchain()
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		r_deviceContext.physicalDevice,
		r_deviceContext.surface,
		&capabilities
	);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		r_deviceContext.physicalDevice,
		r_deviceContext.surface,
		&formatCount,
		nullptr
	);
	if (formatCount == 0)
	{
		clz::log::error("Uhh.... Selected gpu does not have any format lol");
		clz::log::error("Uhh.... Selected gpu does not have any format lol");
		return false;
	}
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		r_deviceContext.physicalDevice,
		r_deviceContext.surface,
		&formatCount,
		formats.data()
	);

	// 1.2 get presentation modes;
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		r_deviceContext.physicalDevice,
		r_deviceContext.surface,
		&presentModeCount,
		nullptr
	);
	if (presentModeCount == 0)
	{
		clz::log::error(
			"renderer/initializers.cpp: Selected physical device does "
			"not have a valid presentation mode"
		);

		return false;
	}
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		r_deviceContext.physicalDevice,
		r_deviceContext.surface,
		&presentModeCount,
		presentModes.data()
	);

	// Choosing the right things for our swapchain

	r_swapchainContext.format = formats[0];
	for (const auto& availableFormat : formats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
		    availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			r_swapchainContext.format = availableFormat;
			break;
		}
	}

	// Primarily select mailbox
	r_swapchainContext.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (clz::config::getBool("window", "triplebuffering", false))
	{

		for (const auto& availablePresentMode : presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				r_swapchainContext.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
		}
	}

	// Extent
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		r_swapchainContext.extent = capabilities.currentExtent;
	}
	else
	{
		int width, height;
		clz::window::getFramebufferExtents(&width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		r_swapchainContext.extent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		r_swapchainContext.extent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);
	}

	// Finally creating the swapchain
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (imageCount > capabilities.maxImageCount && capabilities.maxImageCount > 0)
		imageCount = capabilities.maxImageCount;
	clz::log::debug("max image count: " + std::to_string(imageCount));

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = r_deviceContext.surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = r_swapchainContext.format.format;
	swapchainInfo.imageColorSpace = r_swapchainContext.format.colorSpace;
	swapchainInfo.imageExtent = r_swapchainContext.extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	uint32_t QueueFamilyIndices[] = {
		r_deviceContext.graphicsFamily.value(),
		r_deviceContext.presentFamily.value()
	};
	if (r_deviceContext.graphicsFamily != r_deviceContext.presentFamily)
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = QueueFamilyIndices;
	}
	else
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = nullptr;
	}
	swapchainInfo.preTransform = capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = r_swapchainContext.presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(
		    r_deviceContext.device,
		    &swapchainInfo,
		    nullptr,
		    &r_swapchainContext.swapchain
	    ) != VK_SUCCESS) [[unlikely]]
	{
		clz::log::error("could not create swapchain sadly :(");
		return false;
	}

	vkGetSwapchainImagesKHR(
		r_deviceContext.device,
		r_swapchainContext.swapchain,
		&imageCount,
		nullptr
	);

	r_swapchainContext.images.resize(imageCount);

	vkGetSwapchainImagesKHR(
		r_deviceContext.device,
		r_swapchainContext.swapchain,
		&imageCount,
		r_swapchainContext.images.data()
	);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		setHandleName(
			reinterpret_cast<uint64_t>(r_swapchainContext.images[i]),
			VK_OBJECT_TYPE_IMAGE,
			("swapchain image: " + std::to_string(i)).c_str()
		);
	}

	// retrieving image views
	r_swapchainContext.imageViews.resize(r_swapchainContext.images.size());
	for (size_t i = 0; i < r_swapchainContext.images.size(); ++i)
	{
		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = r_swapchainContext.images[i];
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = r_swapchainContext.format.format;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(
			    r_deviceContext.device,
			    &imageViewInfo,
			    nullptr,
			    &r_swapchainContext.imageViews[i]
		    ) != VK_SUCCESS)
		{
			clz::log::error("Could not create swapchain image views");
			return false;
		}
		setHandleName(
			reinterpret_cast<uint64_t>(r_swapchainContext.imageViews[i]),
			VK_OBJECT_TYPE_IMAGE_VIEW,
			("swapchainImageView" + std::to_string(i)).c_str()
		);
	}

	clz::log::info("renderer: created swapchain");

	setHandleName(
		reinterpret_cast<uint64_t>(r_swapchainContext.swapchain),
		VK_OBJECT_TYPE_SWAPCHAIN_KHR,
		"swapchain"
	);

	return true;
}

VkFormat findSupportedFormat(
	const std::vector<VkFormat>& candidates,
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
bool createDepthResources()
{
	r_swapchainContext.depthFormat = findSupportedFormat(
		{VK_FORMAT_D32_SFLOAT},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	if (r_swapchainContext.depthFormat == VK_FORMAT_UNDEFINED)
	{
		clz::log::error("Could not find depth format");
		return false;
	}

	if (!clz::renderer::createImage(
		    r_swapchainContext.depthImage,
		    "Depth Image",
		    r_swapchainContext.extent.width,
		    r_swapchainContext.extent.height,
		    r_swapchainContext.depthFormat,
		    VK_IMAGE_TILING_OPTIMAL,
		    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		    0
	    ))
	{
		clz::log::error("Could not create depth image");
		return false;
	}
	setHandleName(
		reinterpret_cast<uint64_t>(r_swapchainContext.depthImage),
		VK_OBJECT_TYPE_IMAGE,
		"swapchain depth image"
	);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(
		clz::renderer::r_deviceContext.device,
		r_swapchainContext.depthImage,
		&memRequirements
	);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memRequirements.size;
	memAllocInfo.memoryTypeIndex = findMemoryType(
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	if (vkAllocateMemory(
		    clz::renderer::r_deviceContext.device,
		    &memAllocInfo,
		    nullptr,
		    &r_swapchainContext.depthDeviceMemory
	    ) != VK_SUCCESS)
	{
		clz::log::error("vulkan could not create depth memory");
		return false;
	}
	setHandleName(
		reinterpret_cast<uint64_t>(r_swapchainContext.depthDeviceMemory),
		VK_OBJECT_TYPE_DEVICE_MEMORY,
		"swapchain depth memory"
	);

	vkBindImageMemory(
		clz::renderer::r_deviceContext.device,
		r_swapchainContext.depthImage,
		r_swapchainContext.depthDeviceMemory,
		0
	);

	if (!createImageView(
		    r_swapchainContext.depthImageView,
		    "Depth image view",
		    r_swapchainContext.depthImage,
		    r_swapchainContext.depthFormat,
		    VK_IMAGE_ASPECT_DEPTH_BIT
	    ))
	{
		clz::log::error("Could not create depth image view");
		return false;
	}
	setHandleName(
		reinterpret_cast<uint64_t>(r_swapchainContext.depthImageView),
		VK_OBJECT_TYPE_IMAGE_VIEW,
		"swapchain depth image view"
	);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = clz::renderer::r_commandContext.commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = startSingleTimeCommand();

	transition_image_layout(
		r_swapchainContext.depthImage,
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

	submitSingleTimeCommand(commandBuffer);

	clz::log::info("Created depth resources");
	return true;
}

void recreateSwapchainContext()
{
	vkDeviceWaitIdle(r_deviceContext.device);
	destroySwapchainContext();

	if (!initSwapchainContext()) [[unlikely]]
	{
		clz::log::error("Mid loop, failed to recreate swapchain");
	}
}
} // namespace clz::renderer

namespace clz::renderer
{
void destroySwapchainContext()
{
	destroyDepthResources();
	destroySwapchain();
	clz::log::info("destroyed swapchain context");
}

void destroySwapchain()
{
	for (const auto& imageView : r_swapchainContext.imageViews)
	{
		vkDestroyImageView(r_deviceContext.device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(r_deviceContext.device, r_swapchainContext.swapchain, nullptr);

	clz::log::info("destroyed swapchain");
}

void destroyDepthResources()
{
	vkDestroyImageView(
		r_deviceContext.device,
		r_swapchainContext.depthImageView,
		nullptr
	);
	vkDestroyImage(r_deviceContext.device, r_swapchainContext.depthImage, nullptr);
	vkFreeMemory(r_deviceContext.device, r_swapchainContext.depthDeviceMemory, nullptr);

	clz::log::info("destroyed depth resources");
}
} // namespace clz::renderer
