/**
 * @file camerafunctions.hpp
 * @author curl0z
 * @brief This file contains all camera local functions
 */
#pragma once

#include "cameradata.hpp"
#include <string>

namespace clz::renderer::camera
{
	/**
	 * @brief Initialize/load a camera
	 * @param name camera name
	 * @param id camera ID
	 * @return true on success, false otherwise and logs an error
	 */
	bool loadCamera(const std::string& name, const CameraID id);

	/**
	 * @brief Processes keyboard input for any camera actions
	 * @param id camera ID
	 */
	void processKeyBoardInput(const CameraID id);

	/**
	 * @brief Computer forward and right vectors for camera @p id
	 * @param id camera id
	 */
	void updateCameraVectors(const CameraID id);

	/**
	 * @brief Updates pitch/yaw for camera @p id from raw cursor position.
	 * @param id Camera ID
	 * @param xPos Pointer's x-coordinate in screen.
	 * @param yPos Pointer's y-coordinate in screen.
	 */
	void processMouseInput(const CameraID id, const float xPos, const float yPos);

	/**
	 * @brief Adjusts FOV for camera @p id from scroll input, clamped to [1, 89] degrees.
	 * @param id Camera ID
	 * @param yOffset cursor scroll offset
	 */
	void processMouseScroll(const CameraID id, const float yOffset);

} // namespace clz::renderer::camera