/**
 * @file camera.hpp
 * @author curl0z
 * @brief Camera main header file
 */
#pragma once

#include "cameradata.hpp"
#include "math/mat4x4.hpp"
#include <optional>

/// --- data ---
namespace clz::renderer
{
	// --- The Definition Struct ---
	struct CameraDef
	{
		// Core properties
		float fov = 60.0f;
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
		float maxVelocity = 5.0f;
		float sensitivity = 0.1f;
		float acceleration = 32768.0f;

		// Orientation
		float pitch = 0.0f; // degrees
		float yaw = -90.0f;   // degrees

		// Transform
		math::vec3 position = math::vec3(0.0f);

		/// OUTDATE THIS
		math::vec3 localFront;
	};

} // namespace clz::renderer
namespace clz::renderer
{
	/// @brief The Create Function
	/// @param def Camera defintion object
	CameraId createCamera(const CameraDef& def);

	/// @brief Updates camera data, through a new definition value.
	/// @param cameraId Id of camera whose data is to be updated.
	/// @param def New camera definition data.
	void updateCameraData(const CameraId cameraId, const CameraDef& def);

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

	/// @brief Returns a point along the active camera's view direction, for lookAt
	/// targets.
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

	/// @brief Retrieves camera's current velocity
	/// @param Id ID of camera
	math::vec3 getCameraVelocity(CameraId Id);

	/// @brief Retrieves camera's local front vector
	/// @param Id ID of camera
	math::vec3 getCameraLocalFrontVector(CameraId Id);
	/// @brief Retrieves camera's local right vector
	/// @param Id ID of camera
	math::vec3 getCameraLocalRightVector(CameraId Id);

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
} // namespace clz::renderer
