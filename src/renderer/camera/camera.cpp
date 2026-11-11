/**
 * @file camera.cpp
 * @author curl0z
 * @brief Camera's main implementation file
 */

#include "renderer/camera/camera.hpp"
#include "core/logs.hpp"
#include "math/angle.hpp"
#include "math/vec2.hpp"
#include "math/worldtransform.hpp"
#include "renderer/camera/cameradata.hpp"
#include "renderer/camera/camerafunctions.hpp"
#include "renderer/vk_types.hpp"
#include "window/mouse.hpp"
#include "core/assert.hpp"

namespace clz::renderer
{
	/// @copydoc
	CameraId createCamera(const CameraDef& def)
	{
		Near.push_back(def.nearPlane);
		Far.push_back(def.farPlane);

		MaxVelocity.push_back(def.maxVelocity);
		Velocity.emplace_back(0.0f);
		Sensitivity.push_back(def.sensitivity);
		Acceleration.push_back(def.acceleration);

		Pitch.push_back(def.pitch);
		Yaw.push_back(def.yaw);

		Fov.push_back(def.fov);

		const auto pos = def.position;
		const auto frnt = def.localFront;
		Position.push_back(pos);
		LocalFront.push_back(frnt);
		LocalRight.push_back(math::normalize(math::cross(frnt, WorldUp)));

		ProjMatrix.emplace_back(1.0f);
		ChangeProjMatrix.push_back(true);

		return NumCameras++;
	}


	void updateCameraData(const CameraId cameraId, const CameraDef& def)
	{
#ifdef CLZ_ENABLE_CHECKS /***************************************************/
		CLZ_ASSERT(
		    cameraId < NumCameras,
		    "Attempt to update out-of-bound/non-existent camera"
		);
#endif /*********************************************************************/

		Near[cameraId] = def.nearPlane;
		Far[cameraId] = def.farPlane;

		MaxVelocity[cameraId] = def.maxVelocity;
		Velocity[cameraId] = math::vec3(0.0f);

		Sensitivity[cameraId] = def.sensitivity;
		Acceleration[cameraId] = def.acceleration;

		Pitch[cameraId] = def.pitch;
		Yaw[cameraId] = def.yaw;

		Fov[cameraId] = def.fov;

		Position[cameraId] = def.position;
		LocalFront[cameraId] = def.localFront;
		LocalRight[cameraId] = math::normalize(math::cross(def.localFront, WorldUp));
		ChangeProjMatrix[cameraId] = true;
	}

	/// @copydoc
	void resetCamera(const CameraId id)
	{
		Position[id] = math::vec3(0.0f, 0.0f, 0.0f);
		LocalFront[id] = math::vec3(0.0f, 0.0f, -1.0f);
		LocalRight[id] = math::vec3(1.0f, 0.0f, 0.0f);
		Pitch[id] = 0.0f;
		Yaw[id] = -90.0f;
		Velocity[id] = math::vec3(0.0f, 0.0f, 0.0f);
		ChangeProjMatrix[id] = true;
	}

	/// @copydoc
	void updateCamera(const CameraId id)
	{
		CLZ_ASSERT(id < NumCameras, "Invalid CameraId");

		processKeyBoardInput(id);
		processMouseInput(id);
		processMouseScroll(id);
	}

	/// @copydoc
	math::vec3 getCameraPosition(const CameraId id)
	{
		CLZ_ASSERT(id < NumCameras, "Invalid CameraId");

		return Position[id];
	}

	/// @copydoc
	void setCameraPosition(const CameraId id, const math::vec3& pos)
	{
		CLZ_ASSERT(id < NumCameras, "Invalid CameraId");

		Position[id] = pos;
	}

	/// @copydoc
	math::vec3 getCameraTarget(const CameraId id)
	{
		CLZ_ASSERT(id < NumCameras, "Invalid CameraId");

		return Position[id] + LocalFront[id];
	}

	/// @copydoc
	float getCameraFov(const CameraId id)
	{
		CLZ_ASSERT(id < NumCameras, "Invalid CameraId");

		return Fov[id];
	}
	/// @copydoc
	void setCameraFov(const CameraId Id, const float newFov)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");

		Fov[Id] = newFov;
	}

	/// @copydoc
	float getCameraNearPlane(const CameraId Id)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");

		return Near[Id];
	}
	/// @copydoc
	void setCameraNearPlane(const CameraId Id, const float newNearPlane)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");
		Near[Id] = newNearPlane;
	}

	/// @copydoc
	float getCameraFarPlane(const CameraId Id)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");
		return Far[Id];
	}
	/// @copydoc
	void setCameraFarPlane(const CameraId Id, const float newFarPlane)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");
		Far[Id] = newFarPlane;
	}

	/// @copydoc getCameraVelocity
	math::vec3 getCameraVelocity(const CameraId Id)
	{
		return Velocity[Id];
	}
	/// @copydoc getCameraLocalFrontVector
	math::vec3 getCameraLocalFrontVector(const CameraId Id)
	{
		return LocalFront[Id];
	}
	/// @copydoc getCameraLocalRightVector
	math::vec3 getCameraLocalRightVector(const CameraId Id)
	{
		return LocalRight[Id];
	}

	/// @copydoc
	float getCameraPitch(const CameraId Id)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");
		return Pitch[Id];
	}
	/// @copydoc
	void setCameraPitch(const CameraId Id, const float pitch)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");
		Pitch[Id] = pitch;
	}
	/// @copydoc
	float getCameraYaw(const CameraId Id)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");

		return Yaw[Id];
	}
	/// @copydoc
	void setCameraYaw(const CameraId Id, const float yaw)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");

		Yaw[Id] = yaw;
	}

	/// @copydoc
	math::mat4 getCameraViewMatrix(const CameraId Id)
	{
		return math::makeViewMatrix(Position[Id], Position[Id] + LocalFront[Id], WorldUp);
	}

	/// @copydoc
	math::mat4 getCameraProjMatrix(const CameraId Id, const float width, const float height)
	{
		if (ChangeProjMatrix[Id])
		{
			ProjMatrix[Id] = math::makePerspectiveMatrix(
				Far[Id],
				Near[Id],
				width / height,
				math::radians(Fov[Id])
			);

			ChangeProjMatrix[Id] = false;
			clz::log::debug("Updated projection matrix");
		}

		return ProjMatrix[Id];
	}

	/// @copydoc
	void updateCameraProjMatrix(const CameraId Id)
	{
		CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");

		ChangeProjMatrix[Id] = true;
	}

} // namespace clz::renderer
