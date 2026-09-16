/**
 * @file swapchaincontext.hpp
 * author curl0z
 * @brief Contains functions for initializing and
 * destroying all the handles in swapchain context.
 */

#pragma once

#include <cstdint>

namespace clz::renderer
{
	inline bool r_enableTripleBuffering;
}
namespace clz::renderer
{
	/**
	 * @brief Initializes the swapchain context.
	 * ie all the device handles.
	 * Can view all of them in context.hpp
	 * @param width Width of swapchain image
	 * @param height Height of swapchain image
	 * @return true on success, false on any other issue
	 */
	bool initSwapchainContext(std::uint32_t width, std::uint32_t height);

	/**
	 * @brief Creates swapchain, initializes all the
	 * images and image views
	 * @param width Width of swapchain image
	 * @param height Height of swapchain image
	 * and sets all the other swapchain settings
	 * @return true on success, false on any other issue
	 */
	bool createSwapchain(std::uint32_t width, std::uint32_t height);

	/**
	 * @brief Recreates the swapchain.
	 * Called when the swapchain becomes invalid
	 * Destroys and rebuilds the whole swapchain context
	 * 
	 * @param width Width of swapchain image
	 * @param height Height of swapchain image
	 * @note window subsystem also calls it internally
	 * whenever window is resized
	 * @return true on success, false on any other issue
	 */
	bool recreateSwapchainContext(std::uint32_t width, std::uint32_t height);
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
