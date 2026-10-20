/**
 * @file render_target_context.hpp
 * @author curl0z
 * @brief Initializes render target context of our application
 */
#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace clz::renderer
{
	/// @brief MSAA value
	/// Is initially set by config data
	inline VkSampleCountFlagBits MSAA = VK_SAMPLE_COUNT_1_BIT;
}

namespace clz::renderer
{
	/// @brief Initializes Render-target context
	/// @param width Width of render target context
	/// @param height Height of render target context
	/// @return True on succesful creation, false otherwise
	bool initRenderTargetContext(uint32_t width, uint32_t height);

	/// @brief Destroy's render target context
	/// @param isRecreating Flag indicating whether this destruction
	/// is for recreation purpose. False by default.
	/// This is done cuz indivisual freeing is slow, so let the arena handle it.
	void destroyRenderTargetContext(const bool isRecreating = false);

	/// @brief Recreates render target context
	/// @param width Width of render target context
	/// @param height Height of render target context
	/// @return True on succesful creation, false otherwise
	bool recreateRenderTargetContext(
		uint32_t width,
		uint32_t height);
}
