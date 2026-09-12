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

	///< @brief Flag set by hintRendererFramebufferResize function
	///< called by window system, whenever window is resized.
	///< @warning Make sure that swapchain is recreated once per frame
	///< Before recording of command buffer has started
	inline bool r_framebufferResized = false;

	///< @brief Enum depicting result of resizing images
	///< on framebuffer resize.
	///< Success defines all images were re-created successfully.
	///< Failure defines somewhere some issue happened.
	///< Not-resized is the case where maybe width or height of framebuffer was 0, 
	///< So we're not resizing this frame.
	enum ImagesResizeResult
	{
		SUCCESS,
		FAILURE,
		INVALID_EXTENTS
	};
	/// @brief recreates all outdated images whenever
	/// window is resized
	/// @warning Make sure that swapchain is recreated once per frame
	/// Before recording of command buffer has started
	/// @return true if each image has been succesfully created
	/// false otherwise.
	ImagesResizeResult recreateImagesOnFramebufferResize();

} // namespace clz::renderer
