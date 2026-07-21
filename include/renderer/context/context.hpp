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

namespace clz::renderer
{
	/**
	 * @struct DeviceContext
	 * @brief Stores Vulkan device-level resources.
	 *
	 * Contains:
	 * - Vulkan instance
	 * - Selected physical device (GPU)
	 * - Logical device
	 * - Rendering surface
	 * - Graphics and present queues
	 * - Queue family indices
	 */
	struct DeviceContext
	{
		VkInstance instance = VK_NULL_HANDLE;			  ///< Vulkan instance
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE; ///< Debug messenger
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;	  ///< Selected physical device
		VkDevice device = VK_NULL_HANDLE;			  ///< Logical device
		VkSurfaceKHR surface = VK_NULL_HANDLE;			  ///< Window surface
		VkQueue graphicsQueue = VK_NULL_HANDLE;			  ///< Graphics queue
		VkQueue presentQueue = VK_NULL_HANDLE;			  ///< Presentation queue
		std::optional<uint32_t> graphicsFamily = {};		  ///< Graphics queue family index
		std::optional<uint32_t> presentFamily = {};		  ///< Present queue family index
	};

	struct GPUInfo
	{
		VkDeviceSize uniformBufferOffsetAlignment;
		float maxAnisotropy;
	};

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

		VkImage depthImage = VK_NULL_HANDLE;
		VkImageView depthImageView = VK_NULL_HANDLE;
		VkFormat depthFormat = VK_FORMAT_UNDEFINED;
		VkDeviceMemory depthDeviceMemory = VK_NULL_HANDLE;
	};

	/**
	 * @struct PipelineContext
	 * @brief Stores graphics pipeline resources.
	 *
	 * Contains:
	 * - Graphics pipeline
	 * - Pipeline layout
	 * - Shader modules
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
	 * @brief Stores per-frame resources.
	 *
	 * Contains:
	 * - Command pool and command buffers
	 * - Synchronization primitives:
	 *   - Image-available semaphores
	 *   - Render-finished semaphores
	 *   - In-flight fences
	 *
	 * Notes:
	 * - Sized according to frames-in-flight or swapchain image count.
	 */
	struct FrameContext
	{
		std::vector<VkSemaphore> renderReadySemaphores = {};  ///< Signals image acquisition
		std::vector<VkSemaphore> presentReadySemaphores = {}; ///< Signals rendering completion
		std::vector<VkFence> inFlightFences = {};	      ///< CPU-GPU synchronization fences
	};
} // namespace clz::renderer
