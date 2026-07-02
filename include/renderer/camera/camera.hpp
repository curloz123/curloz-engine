/**
 * @file camera.hpp
 * @author curl0z
 * @brief Camera main header file
 */
#pragma once

#include "cameradata.hpp"

namespace clz::renderer::camera
{
	/// @brief Switches the active camera and resets its mouse-delta tracking.
	void setActiveCamera(const CameraID id);

	/// @brief Loads game (and editor, if enabled) camera config at startup.
	/// @return true if all required cameras loaded successfully.
	bool initializeCameras();

	/// @brief Drives input handling for the currently active camera.
	void update(const float xPos, const float yPos, const float scroll);

	/// @brief Returns the active camera's world-space position.
	inline math::vec3 getPosition()
	{
		return Position[activeCamera];
	}

	/// @brief Returns a point along the active camera's view direction, for lookAt targets.
	inline math::vec3 getTarget()
	{
		return Position[activeCamera] + localFront[activeCamera];
	}

	/// @brief Returns the active camera's field of view, in degrees.
	inline float getFov()
	{
		return Fov[activeCamera];
	}
}