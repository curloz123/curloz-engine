/**
 * @file swapchaincontext.hpp
 * author curl0z
 * @brief Contains functions for initializing and
 * destroying all the handles in swapchain context.
 */
#pragma once

namespace clz::renderer
{
	/**
	 * @brief Initializes the swapchain context.
	 * ie all the device handles.
	 * Can view all of them in context.hpp
	 */
	bool initSwapchainContext();

	/**
	 * @brief Creates swapchain, initializes all the
	 * images and image views
	 * and sets all the other swapchain settings
	 */
	bool createSwapchain();

	/**
	 * @brief Recreates the swapchain.
	 *
	 * Called when the swapchain becomes invalid
	 * Destroys and rebuilds the whole swapchain context
	 *
	 * @note window subsystem also calls it internally
	 * whenever window is resized
	 */
	void recreateSwapchainContext();
} // namespace clz::renderer

namespace clz::renderer
{
	/**
	 * @brief Destroys the swapchain context.
	 */
	void destroySwapchainContext();

	/**
	 * @brief Destroys the all the handles in
	 * the swapchain context
	 */
	void destroySwapchain();

} // namespace clz::renderer
