/**
 * @file camerafunctions.hpp
 * @author curl0z
 * @brief This file contains all camera local functions
 */
#pragma once

#include "cameradata.hpp"

/// --- camera functions ---
namespace clz::renderer
{
	/**
	 * @brief Processes keyboard input for any camera actions
	 * @param id camera ID
	 */
	void processKeyBoardInput(CameraId id);

	/**
	 * @brief Updates pitch/yaw for camera @p id from raw cursor position.
	 * @param id Camera ID
	 */
	void processMouseInput(CameraId id);

	/**
	 * @brief Adjusts FOV for camera @p id from scroll input, clamped to [1, 89] degrees.
	 * @param id Camera ID
	 */
	void processMouseScroll(CameraId id);

	/**
	 * @brief Computer forward and right vectors for camera @p id
	 * @param id camera id
	 */
	void updateCameraVectors(CameraId id);

} // namespace clz::renderer