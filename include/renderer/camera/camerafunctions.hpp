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
 * @param xPos Pointer's x-coordinate in screen.
 * @param yPos Pointer's y-coordinate in screen.
 */
void processMouseInput(CameraId id, float xPos, float yPos);

/**
 * @brief Adjusts FOV for camera @p id from scroll input, clamped to [1, 89] degrees.
 * @param id Camera ID
 * @param yOffset cursor scroll offset
 */
void processMouseScroll(CameraId id, float yOffset);

/**
 * @brief Computer forward and right vectors for camera @p id
 * @param id camera id
 */
void updateCameraVectors(CameraId id);

} // namespace clz::renderer