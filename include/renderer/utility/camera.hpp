#pragma once

#include "math/vec3.hpp"
#include <vector>
#include <cstdint>

/// --- Data ---
namespace clz::renderer
{
	/// @brief Index type used to select a camera slot.

	/// @brief Number of cameras
	inline uint8_t NumCameras = 0;

	/// @brief Shared world-space up reference used for basis derivation.
	inline const math::vec3 WorldUp = math::vec3(0.0f, 1.0f, 0.0f);

	/// @brief Near and Far cap values of camera
	inline std::vector<float> Near;
	inline std::vector<float> Far;

	/// @brief Per-camera max movement speed.
	inline std::vector<float> MaxVelocity;
	/// @brief Per-camera current velocity vector.
	inline std::vector<math::vec3> Velocity;
	/// @brief Per-camera sensitivity.
	inline std::vector<float> Sensitivity;
	/// @brief Per-camera acceleration/friction rate.
	inline std::vector<float> Acceleration;

	/// @brief Per-camera pitch, in degrees.
	inline std::vector<float> Pitch;
	/// @brief Per-camera yaw, in degrees.
	inline std::vector<float> Yaw;

	/// @brief Per-camera field of view, in degrees.
	inline std::vector<float> Fov;

	/// @brief Per-camera world-space position.
	inline std::vector<math::vec3> Position;
	/// @brief Per-camera forward vector, derived from pitch/yaw.
	inline std::vector<math::vec3> LocalFront;
	/// @brief Per-camera right vector, derived from forward x WorldUp.
	inline std::vector<math::vec3> LocalRight;

	/// @brief Hints that projection matrix should be recalculated
	inline std::vector<bool> ChangeProjMatrix;

	/// @brief Per-camera last recorded cursor X, for mouse delta calc.
	inline std::vector<float> LastX;
	/// @brief Per-camera last recorded cursor Y, for mouse delta calc.
	inline std::vector<float> LastY;

	/// @brief Per-camera flag to avoid a mouse-delta snap on first use.
	inline std::vector<bool> FirstTime;
}

/// --- helper functions
namespace clz::renderer
{

}
