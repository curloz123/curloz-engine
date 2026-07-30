/**
 * @file camera.hpp
 * @author curl0z
 * @brief Camera main header file
 */
#pragma once

#include "cameradata.hpp"
#include "math/mat4x4.hpp"
#include "core/assert.hpp"

/// --- data ---
namespace clz::renderer
{
	// --- The Definition Struct ---
	struct CameraDef
	{
		// Core properties
		std::optional<float> fov;
		std::optional<float> nearPlane;
		std::optional<float> farPlane;
		std::optional<float> maxVelocity;
		std::optional<float> sensitivity;
		std::optional<float> acceleration;

		// Orientation
		std::optional<float> pitch;  // degrees
		std::optional<float> yaw;    // degrees

		// Transform
		std::optional<math::vec3> position;

		// Derived vectors (typically set from pitch/yaw, but you want defaults if missing)
		std::optional<math::vec3> localFront;
	};

}
namespace clz::renderer
{
	/// @brief The Create Function
	/// @param def Camera defintion object
	CameraId createCamera(const CameraDef& def);

	/// @brief Hints that we are going to use this camera,
	/// so engine can ready some things in advance
	void useCamera(CameraId id);

	/// @brief Drives input handling
	/// For the currently active camera and manage mode switching
	/// @param id Camera ID
	void updateCamera(CameraId id);

	/// @brief resets all the data of camera to default
	/// @param id Camera ID
	void resetCamera(CameraId id);

	/// @brief Returns the active camera's world-space position.
	/// @param id Camera ID
	/// @return Camera Position in vec3 format
	math::vec3 getCameraPosition(CameraId id);
	/// @brief Sets camera position
	/// @param id Id of camera
	/// @param pos New Camera position
	void setCameraPosition(CameraId id, const math::vec3& pos);

	/// @brief Returns a point along the active camera's view direction, for lookAt targets.
	/// @param id Camera ID
	/// @return Camera's target/world-front vector
	math::vec3 getCameraTarget(CameraId id);

	/// @brief Returns the active camera's field of view, in degrees.
	/// @param id Camera ID
	/// @return Fov of camera in float
	float getCameraFov(CameraId id);
	/// @brief Sets fov for a camera
	/// @param Id camera ID
	/// @param newFov New Fov of camera
	void setCameraFov(CameraId Id, float newFov);

	/// @brief Returns camera near plane value
	/// @param Id Camera Id
	/// @return Near plane value of camera in float
	float getCameraNearPlane(CameraId Id);
	/// @brief Sets camera near plane value
	/// @param Id Camera id
	/// @param newNearPlane New near plane value
	void setCameraNearPlane(CameraId Id, float newNearPlane);

	/// @brief Returns camera far plane value
	/// @param Id Camera Id
	/// @return Far plane value of camera in float
	float getCameraFarPlane(CameraId Id);
	/// @brief Sets camera far plane value
	/// @param Id Camera id
	/// @param newFarPlane New near plane value
	/// @return Far Plane value of camera
	void setCameraFarPlane(CameraId Id, float newFarPlane);

	/// @brief Gets camera pitch
	/// @param Id Camera id
	/// @return pitch of given camera
	float getCameraPitch(CameraId Id);
	/// @brief Sets camera pitch
	/// @param Id camera id
	/// @param pitch New pitch value
	void setCameraPitch(CameraId Id, float pitch);
	/// @brief Gets camera yaw
	/// @param Id Camera id
	/// @return yaw of given camera
	float getCameraYaw(CameraId Id);
	/// @brief Sets camera yaw
	/// @param Id camera id
	/// @param yaw New yaw value
	void setCameraYaw(CameraId Id, float yaw);

	/// @brief Retrieves view matrix for a camera
	/// @param Id Camera Id
	/// @return View matrix of camera as a math::mat4
	math::mat4 getCameraViewMatrix(CameraId Id);
	/// @brief Retrieves projection matrix for a camera
	/// @param Id Camera Id
	/// @return Projection matrix of camera as a math::mat4
	math::mat4 getCameraProjMatrix(CameraId Id, float width, float height);
	/// @brief Hints engine to update this camera's projection matrix
	/// @param Id Camera Id
	void updateCameraProjMatrix(CameraId Id);

	/// @brief Enable camera'a first time flag
	/// @param Id Camera Id
	void setCameraFirstTime(CameraId Id);
} // namespace clz::renderer::camera