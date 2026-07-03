/**
 * @file renderer.hpp
 * @author curl0z
 * @brief Public interface for renderer subsystem
 *
 * Wraps all the vulkan functions into subsystem functions
 *
 * @note init function must be called after window and config has been initialized
 */

#pragma once

namespace clz::renderer
{
	/**
	 * @brief Initializes all the vulkan handles divided into context's
	 * @return true if everything went well, else calls clz::log::error and return false
	 * @note logs an error and returns on early failure
	 */
	bool init();

	/**
	 * @brief updates the renderer
	 *
	 * @note logs an error if swapchain is outdated or some rare event happens
	 */
	void update();

	/**
	 * @brief Destroys all the vulkan context's
	 */
	void shutdown();

	/**
	 * @brief A flag that checks if
	 * window was resized or not
	 *
	 * @warning SHOULD ONLY BE CALLED BY WINDOW SUBSYSTEM's
	 * callback function defined inside window/vulkanhelper.hpp
	 * or when swapchain goes out of date or suboptimal
	 */
	inline bool r_recreateSwapchain = false;
} // namespace clz::renderer
