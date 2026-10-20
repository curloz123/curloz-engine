/**
 * @file context.hpp
 * @author curl0z
 * @brief Core Vulkan context structures used by the renderer.
 *
 * Defines the main data containers for managing Vulkan resources:
 * - Device (instance, GPU, queues)
 * - Swapchain (images, format, extent)
 * - Pipeline (shaders, pipeline state)
 * - Frame (per-frame synchronization and command buffers)
 *
 * @note These are plain data structures with no ownership logic.
 * Creation and destruction are handled by initializer/cleaner modules.
 */
#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include "../arena.hpp"

namespace clz::renderer
{
	/**
	 * @struct DeviceContext
	 * @brief Stores Vulkan device level resources.
	 *
	 * Contains:
	 * 1. Vulkan instance
	 * 2. Selected physical device
	 * 3. Logical device
	 * 4. Rendering surface
	 * 5. Graphics and present queues
	 * 6. Queue family indices
	 */
	struct DeviceContext
	{
		///< Vulkan instance
		VkInstance instance = VK_NULL_HANDLE;
		///< Debug messenger
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
		///< Selected physical device
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		///< Logical device
		VkDevice device = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;		  ///< Window surface
		VkQueue graphicsQueue = VK_NULL_HANDLE;		  ///< Graphics queue
		VkQueue presentQueue = VK_NULL_HANDLE;		  ///< Presentation queue
		std::optional<uint32_t> graphicsFamily{};	  ///< Graphics queue family index
		std::optional<uint32_t> presentFamily{};	  ///< Present queue family index
	};


	/// @brief Command context of vulkan application
	/// Consists of pool and a command buffers as of version 0.7.0
	struct CommandContext
	{
		VkCommandPool commandPool = VK_NULL_HANDLE;	 ///< Command pool
		std::vector<VkCommandBuffer> commandBuffer = {}; ///< Command buffers per image
	};

	/**
	 * @struct SwapchainContext
	 * @brief Stores swapchain-related resources.
	 *
	 * Contains:
	 * - Swapchain handle
	 * - Surface format and present mode
	 * - Swapchain extent (resolution)
	 * - Swapchain images and corresponding image views
	 */
	struct SwapchainContext
	{
		VkSwapchainKHR swapchain = VK_NULL_HANDLE; ///< Swapchain handle
		VkSurfaceFormatKHR format;		   ///< Selected surface format
		VkPresentModeKHR presentMode;		   ///< Presentation mode
		VkExtent2D extent;			   ///< Swapchain resolution
		std::vector<VkImage> images;		   ///< Swapchain images
		std::vector<VkImageView> imageViews;	   ///< Image views for rendering
	};

	/**
	 * @struct RenderTargetContext
	 * @brief Stores the render target resources
	 * Consists of ->
	 * 1. Render target image and its resources
	 * 2. Msaa target and its resources
	 * 3. Msaa value
	 * 4. Depth resources
	 * 5. Image extent of render target
	 * 6. Image format of render target
	 */
	struct RenderTargetContext
	{
		VkImage image = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		VkSampler imageSampler = VK_NULL_HANDLE;
		clzImageMemory imageMemory;

		VkSampleCountFlagBits msaaFlagBits = VK_SAMPLE_COUNT_1_BIT;
		VkImage msaaImage = VK_NULL_HANDLE;
		VkImageView msaaImageView = VK_NULL_HANDLE;
		VkSampler msaaImageSampler = VK_NULL_HANDLE;
		clzImageMemory msaaImageMemory;

		VkFormat imageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D imageExtent;

		VkImage depthImage = VK_NULL_HANDLE;
		VkImageView depthImageView = VK_NULL_HANDLE;
		VkFormat depthFormat = VK_FORMAT_UNDEFINED;
		clzImageMemory depthImageMemory;
	};

	/**
	 * @struct PipelineContext
	 * @brief Stores graphics pipeline resources.
	 */
	struct PipelineContext
	{
		VkPipeline pipeline = VK_NULL_HANDLE;		///< Graphics pipeline
		VkPipelineLayout layout = VK_NULL_HANDLE;	///< Pipeline layout
		VkShaderModule vertexShader = VK_NULL_HANDLE;	///< Vertex shader module
		VkShaderModule fragmentShader = VK_NULL_HANDLE; ///< Fragment shader module

		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> descriptorSets;
	};

	/**
	 * @struct FrameContext
	 * @brief Stores per frame resources.
	 * @note Sized according to frames in flight or swapchain image count.
	 */
	struct FrameContext
	{
		///< Signals image acquisition
		std::vector<VkSemaphore> renderReadySemaphores{};
		///< Signals rendering completion
		std::vector<VkSemaphore> presentReadySemaphores{}; 
		///< CPU-GPU synchronization fences
		std::vector<VkFence> inFlightFences{};		   
	};
} // namespace clz::renderer
