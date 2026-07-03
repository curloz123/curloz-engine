/**
 * @file mainloop.hpp
 * @brief Per-frame rendering operations for the Vulkan renderer.
 */
#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer
{
	/**
	 * @brief All graphics related operations go over here
	 *
	 * @param commandBuffer commandBuffer to record this on
	 * @param currentFrame current frame index
	 */
	void render(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	/**
	 * @brief Waits for the GPU to finish work associated with a fence.
	 * Ensures that CPU does not overwrite resources still in use by GPU.
	 * @param fence Fence associated with the current frame.
	 */
	void waitForGPU(VkFence fence);

	/**
	 * @brief Acquires the next available swapchain image.
	 * Signals the provided semaphore when the image is ready for rendering.
	 * @param semaphore Semaphore to signal when image is available.
	 * @param rImageIndex Reference to an integer which stores index of the acquired swapchain
	 * image.
	 */
	void acquireNextImage(VkSemaphore semaphore, uint32_t& rImageIndex);

	/**
	 * @brief Resets the fence, basically makes it unsignaled
	 * @param fence Fence associated with current frame.
	 */
	void resetFence(VkFence fence);

	/**
	 * @brief Begins recording a command buffer.
	 * Resets and prepares the command buffer for new commands.
	 * @param commandBuffer Command buffer to begin recording.
	 */
	void startCommandBuffer(VkCommandBuffer commandBuffer);

	/**
	 * @brief Records rendering commands into a command buffer.
	 * @param commandBuffer Command buffer to record into.
	 * @param imageIndex Index of the swapchain image being rendered to.
	 */
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	/**
	 * @brief Submits a command buffer to the graphics queue.
	 * Handles synchronization between image acquisition and rendering completion.
	 *
	 * @param commandBuffer Command buffer to submit.
	 * @param imageAvailableSemaphore Signals when image is ready.
	 * @param renderFinishedSemaphore Signals when rendering is complete.
	 * @param inFlightFence Fence used for synchronization.
	 */
	void submitCommandBuffer(VkCommandBuffer commandBuffer, VkSemaphore imageAvailableSemaphore, VkSemaphore renderFinishedSemaphore,
				 VkFence inFlightFence);

	/**
	 * @brief Presents a rendered image to the swapchain.
	 * Waits on the renderfinished semaphore before presentation.
	 *
	 * @param semaphore Semaphore signaling rendering completion.
	 * @param imageIndex Index of the swapchain image to present.
	 */
	void present(VkSemaphore semaphore, uint32_t imageIndex);
} // namespace clz::renderer