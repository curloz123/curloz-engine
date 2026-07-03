/**
 * @file framecontext.hpp
 * @author curl0z
 * @brief Contains functions for
 * initializing the pipeline context
 *
 * @note must be the last context to be initialized
 */
#pragma once

namespace clz::renderer
{
	/**
	 * @brief Initializes the frame context.
	 * ie all the device handles.
	 * Can view all of them in context.hpp
	 */
	bool initFrameContext();

	/**
	 * @brief Creates a command pool
	 * @return true if successful
	 * else returns false and calls clz::error
	 * @note Must be the first handle to be created
	 * in this context
	 */
	bool createCommandPool();

	/**
	 * @brief Creates all the command buffers
	 * @return true if successful
	 * else returns false and calls clz::error
	 * @note Creates ${number of swapchain images}
	 * number of command buffers.
	 * Must be called after command pool has been created
	 */
	bool createCommandBuffers();

	/**
	 * @brief Creates all the frame sync objects
	 * ie all the semaphores(render finished and image available)
	 * and fences.
	 * @return true if successful
	 * else returns false and calls clz::error
	 */
	bool createSyncObjects();
} // namespace clz::renderer

namespace clz::renderer
{
	/**
	 * @brief Destroys all frame context's
	 */
	void destroyFrameContext();

	/**
	 * @brief Destroys the command pool
	 * automatically destroys all command buffers
	 */
	void destroyCommandPool();

	/**
	 * @brief Destroys all the frame sync objects
	 */
	void destroySyncObjects();

} // namespace clz::renderer
