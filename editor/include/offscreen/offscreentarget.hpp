/**
 * @file offscreentarget.hpp
 * @author curl0z
 * @brief This file handles all offscreen images that the editor might require
 */
#pragma once

#include "vulkan/vulkan.h"

namespace clz::editor
{
	/// @brief Tells if any window presenting offscreen targets
	/// If it is, it will tell renderer to not update its camera
	/// as the offscreen window will be using it.
	inline bool IsCurrentlyShowingOffscreenTargets = false;

	/// @brief A single offscreen render target: color + depth image pair,
	/// sampler, and its registered ImGui descriptor set for display via
	/// ImGui::Image(). Owns all Vulkan resources needed to render a scene
	/// into a texture and present it inside an editor panel.
	struct OffscreenTarget
	{
		/// @brief Whether the owning editor window is currently open/visible.
		bool showTarget = false;

		/// @brief Current pixel dimensions of the render target.
		VkExtent2D extent = {.width = 256, .height = 256};

		/// @brief Color image the scene is rendered into.
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;

		/// @brief Sampler used by ImGui to sample the color image as a texture.
		VkSampler sampler = VK_NULL_HANDLE;

		/// @brief Depth image, required since the shared editor pipeline has depth testing enabled.
		VkImage depthImage = VK_NULL_HANDLE;
		VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
		VkImageView depthImageView = VK_NULL_HANDLE;

		/// @brief ImGui-side texture handle, obtained via ImGui_ImplVulkan_AddTexture.
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	};

	/// @brief The offscreen target used by the RigidBody Shape Editor preview panel.
	inline OffscreenTarget physicsBodyShapeImage{};
}

namespace clz::editor
{
	/// @brief Initializes the offscreen rendering backend and creates all offscreen targets.
	/// @return true on success.
	bool createOffscreenTargets();

	/// @brief Destroys the offscreen backend and all offscreen targets' Vulkan resources.
	void destroyOffscreenTargets();

	/// @brief Destroys and recreates a target's Vulkan resources at a new size.
	/// @param target Target to resize.
	/// @param width New width in pixels.
	/// @param height New height in pixels.
	/// @return true on success.
	bool recreateOffscreenTarget(OffscreenTarget& target, uint32_t width, uint32_t height);

	/// @brief Issues the render passes for every currently-open offscreen target.
	/// @param commandBuffer Active command buffer, must be in recording state.
	void drawOffscreenTargets(VkCommandBuffer commandBuffer);

	/// @brief Draws the ImGui windows for every currently-open offscreen target.
	void presentOffscreenWindows();

	/// @brief Notifies the offscreen backend that entity data is ready to be used for rendering.
	void flagOffscreenTargetsEntitiesLoaded();

	/// @brief Whether any offscreen target window is currently open.
	/// Used to suppress main viewport camera updates while an offscreen
	/// preview camera should have input focus instead.
	bool isCurrentlyShowingOffscreenTargets();
}
