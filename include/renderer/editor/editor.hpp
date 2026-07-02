/**
 * @file editor.hpp
 * @author curl0z
 *
 * @brief ImGui sandbox editor
 * for runtime development
 *
 * @note Must be initialized after all renderer's
 * context has been initialized.
 */
#pragma once

#include <vulkan/vulkan.h>

namespace clz::editor
{
	/// @brief Initializes ImGui, descriptor pool, fonts, and Vulkan/GLFW backends.
	/// @return true on success.
	bool init();

	/// @brief Builds all ImGui widget calls for the current frame.
	void render();

	/// @brief Drives the full ImGui frame: NewFrame, render, Render, RenderDrawData.
	/// @param commandBuffer Active command buffer, must be in recording state.
	void update(VkCommandBuffer commandBuffer);

	/// @brief Shuts down ImGui backends, destroys context and descriptor pool.
	void shutdown();
}