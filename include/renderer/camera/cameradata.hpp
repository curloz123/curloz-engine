/**
 * @brief cameradata.hpp
 * @author curl0z
 * @brief Contains all camera's data
 */
#pragma once

#include <cstdint>
#include <array>
#include "math/vec3.hpp"

namespace clz::renderer::camera
{
	/// @brief Index type used to select a camera slot.
	using CameraID = uint8_t;
	/// @brief Total number of camera slots (game + editor).
	constexpr uint8_t NumCameras = 2;

	/// @brief Index of the gameplay camera.
	constexpr CameraID GameCam   = 0;
	/// @brief Index of the editor/free-fly camera.
	constexpr CameraID EditorCam = 1;

	/// @brief Shared world-space up reference used for basis derivation.
	const auto WorldUp = math::vec3(0.0f, 1.0f, 0.0f);

	/// @brief Per-camera max movement speed.
	inline std::array<float, NumCameras> MaxVelocity = { 5.0f, 8.0f };
	/// @brief Per-camera mouse-look sensitivity.
	inline std::array<float, NumCameras> Sensitivity = { 0.4f, 0.4f };
	/// @brief Per-camera movement acceleration/friction rate.
	inline std::array<float, NumCameras> Acceleration = { 15.0f, 20.0f };

	/// @brief Per-camera pitch, in degrees.
	inline std::array<float, NumCameras> Pitch = { 0.0f, 0.0f };
	/// @brief Per-camera yaw, in degrees.
	inline std::array<float, NumCameras> Yaw   = { -90.0f, -90.0f };

	/// @brief Per-camera world-space position.
	inline std::array<math::vec3, NumCameras> Position = {
		math::vec3(0.0f, 0.0f, 0.0f),
		math::vec3(0.0f, 0.0f, 0.0f)
	};
	/// @brief Per-camera forward vector, derived from pitch/yaw.
	inline std::array<math::vec3, NumCameras> localFront = {
		math::vec3(0.0f, 0.0f, -1.0f),
		math::vec3(0.0f, 0.0f, -1.0f)
	};
	/// @brief Per-camera right vector, derived from forward x WorldUp.
	inline std::array<math::vec3, NumCameras> Right = {
		math::vec3(1.0f, 0.0f, 0.0f),
		math::vec3(1.0f, 0.0f, 0.0f)
	};

	/// @brief Per-camera field of view, in degrees.
	inline std::array<float, NumCameras> Fov = { 60.0f, 60.0f };
	inline std::array<bool, NumCameras>  FovChanged = { true, true };

	/// @brief Per-camera current velocity vector.
	inline std::array<math::vec3, NumCameras> Velocity = {
		math::vec3(0.0f), math::vec3(0.0f)
	};

	/// @brief Per-camera last recorded cursor X, for mouse delta calc.
	inline std::array<float, NumCameras> LastX = { 0.0f, 0.0f };

	/// @brief Per-camera last recorded cursor Y, for mouse delta calc.
	inline std::array<float, NumCameras> LastY = { 0.0f, 0.0f };

	/// @brief Per-camera flag to avoid a mouse-delta snap on first use.
	inline std::array<bool, NumCameras>  FirstTime = { true, true };

	/// @brief Currently active camera slot.
	inline CameraID activeCamera = GameCam;

}