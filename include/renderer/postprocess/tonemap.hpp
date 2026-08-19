#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include "core/logs.hpp"

namespace clz::renderer::post_process
{
	inline constexpr VkFormat TONEMAP_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
	inline VkImage tonemapImage;
	inline VkImageView tonemapImageView;
	inline VkSampler tonemapSampler;
	inline VkDeviceMemory tonemapMemory;

	bool createTonemapProcess();
	void destroyTonemapProcess();
	void applyTonemapProcess(VkCommandBuffer commandBuffer);

	// Exposure
	inline std::optional<float> exposure;	
	inline void setExposure(const float newExposure)
	{
		exposure = newExposure;
	}
	inline float getExposure()
	{
		if (exposure.has_value()) [[likely]]
		{
			return exposure.value();	
		}

		clz::log::warn("Post process's 'exposure' queried, before it was even set");
		return 1.0;
	}
}
