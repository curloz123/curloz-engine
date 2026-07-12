#pragma once

#include "vulkan/vulkan.h"

namespace clz::editor
{
	/// @brief Tells if any window presenting offscreen targets
	/// If it is, it will tell renderer to not update its camera
	/// as the offscreen window will be using it
	inline bool IsCurrentlyShowingOffscreenTargets = false;

	struct OffscreenTarget
	{
		bool showTarget = false;
		VkExtent2D extent = {.width = 256, .height = 256};
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		VkImage depthImage = VK_NULL_HANDLE;
		VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
		VkImageView depthImageView = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	};

	inline OffscreenTarget physicsBodyShapeImage{};
}

namespace clz::editor
{
	bool createOffscreenTargets();
	void destroyOffscreenTargets();
	bool recreateOffscreenTarget(OffscreenTarget& target, uint32_t width, uint32_t height);

	void drawOffscreenTargets(VkCommandBuffer commandBuffer);
	void presentOffscreenWindows();

	void flagOffscreenTargetsEntitiesLoaded();

	bool isCurrentlyShowingOffscreenTargets();
}
