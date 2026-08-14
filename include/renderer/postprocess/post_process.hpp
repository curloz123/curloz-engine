#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer::post_process
{
	constexpr inline VkFormat postProcessImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

	inline VkImage postProcessImage = VK_NULL_HANDLE;
	inline VkDeviceMemory postProcessImageMemory = VK_NULL_HANDLE;
	inline VkImageView postProcessImageView = VK_NULL_HANDLE;

	bool initializePostProcesses();
	void applyPostProcessing();
	void destroyPostProcesses();
}