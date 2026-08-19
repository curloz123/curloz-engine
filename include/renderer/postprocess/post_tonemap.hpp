#pragma once

#include "math/vec2.hpp"
#include <vulkan/vulkan.h>
#include <optional>
#include "core/logs.hpp"
#include <algorithm>

namespace clz::renderer::post_process
{
	inline constexpr VkFormat POST_TONEMAP_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
	inline VkImage postTonemapImage;
	inline VkImageView postTonemapImageView;
	inline VkSampler postTonemapSampler;
	inline VkDeviceMemory postTonemapMemory;

	bool createPostTonemapProcess();
	void destroyPostTonemapProcess();

	/// @brief Runs the post-tonemap pass, writing into postTonemapImage.
	/// @note Leaves postTonemapImage in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	/// ready to be copied/blitted into the swapchain or editor viewport image.
	void applyPostTonemapProcess(VkCommandBuffer commandBuffer);


	enum PostTonemapEffects : uint32_t
	{
		Vignette		= 1 << 0,
		ChromaticAbberation 	= 1 << 1
	};

	inline bool enableVignette = false;
	inline std::optional<float> vignetteStart;
	inline std::optional<float> vignetteEnd;
	inline bool isVignetteEnabled()
	{
		return enableVignette;
	}
	inline void toggleVignette(const bool enable)
	{
		enableVignette = enable;
	}
	inline math::vec2 getVignette()
	{
		if (vignetteStart.has_value() && vignetteEnd.has_value()) [[likely]]
		{
			return math::vec2(vignetteStart.value(), vignetteEnd.value());
		}

		clz::log::warn("vignette values queried, even tho they are not set");
		return math::vec2(0.3f, 0.7f);
	}
	inline void setVignette(float start, float end)
	{
		start = std::clamp(start, 0.0f, 1.0f);
		end   = std::clamp(end, 0.0f, 1.0f);

		vignetteStart 	= start;
		vignetteEnd 	= end;
	}
	inline void setVignette(const math::vec2& vignette)
	{
		vignetteStart = vignette.x;
		vignetteEnd = vignette.y;
	}

	inline bool enableChromaticAberration = false;
	inline std::optional<float> chromaticAbberationStrength;
	inline bool isChromaticAberrationEnabled()
	{
		return enableChromaticAberration;
	}
	inline void toggleChromaticAberration(const bool enable)
	{
		enableChromaticAberration = enable;
	}
	inline float getChromaticAberrationStrength()
	{
		if (chromaticAbberationStrength.has_value()) [[likely]]
			return chromaticAbberationStrength.value();

		clz::log::warn("chromatic aberration value queried, even tho its not set");
		return 0.002f;
	}
	inline void setChromaticAberrationStrength(float value)
	{
		// value = std::clamp(value, -0.01f, 0.01f);

		chromaticAbberationStrength = value;
	}
}
