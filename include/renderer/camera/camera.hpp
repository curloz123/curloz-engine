/**
 * @file camera.hpp
 * @author curl0z
 * @brief Camera main header file
 */
#pragma once

#include "cameradata.hpp"
#include "cameramatrices.hpp"

namespace clz::renderer::camera
{
	/// @brief Switches the active camera and resets its mouse-delta tracking.
	void setActiveCamera(CameraID id);

	/// @brief Drives input handling for the currently active camera and manage mode switching
	void update();

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

	/// @brief Hint's camera that projection matrix should be recalculated
	inline void updateProjectionMatrix()
	{
		ProjMatrixChanged[activeCamera] = true;
	}
} // namespace clz::renderer::camera