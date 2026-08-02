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

namespace clz::renderer
{
/// @copydoc
CameraId createCamera(const CameraDef& def)
{
	constexpr float DEFAULT_FOV = 60.0f;
	constexpr float DEFAULT_NEAR = 0.1f;
	constexpr float DEFAULT_FAR = 100.0f;
	constexpr float DEFAULT_MAX_VELOCITY = 5.0f;
	constexpr float DEFAULT_SENSITIVITY = 0.1f;
	constexpr float DEFAULT_ACCELERATION = 32768.0f;
	constexpr float DEFAULT_PITCH = 0.0f;
	constexpr float DEFAULT_YAW = -90.0f;
	const math::vec3 DEFAULT_POSITION = {0.0f, 0.0f, 0.0f};
	const math::vec3 DEFAULT_FRONT = {0.0f, 0.0f, -1.0f};
	const math::vec3 DEFAULT_RIGHT = {1.0f, 0.0f, 0.0f};

	Near.push_back(def.nearPlane.value_or(DEFAULT_NEAR));
	Far.push_back(def.farPlane.value_or(DEFAULT_FAR));

	MaxVelocity.push_back(def.maxVelocity.value_or(DEFAULT_MAX_VELOCITY));
	Velocity.emplace_back(math::vec3(0.0f));
	Sensitivity.push_back(def.sensitivity.value_or(DEFAULT_SENSITIVITY));
	Acceleration.push_back(def.acceleration.value_or(DEFAULT_ACCELERATION));

	Pitch.push_back(def.pitch.value_or(DEFAULT_PITCH));
	Yaw.push_back(def.yaw.value_or(DEFAULT_YAW));

	Fov.push_back(def.fov.value_or(DEFAULT_FOV));

	const auto pos = def.position.value_or(DEFAULT_POSITION);
	const auto frnt = def.localFront.value_or(DEFAULT_FRONT);
	Position.push_back(pos);
	LocalFront.push_back(frnt);
	LocalRight.push_back(math::normalize(math::cross(frnt, WorldUp)));

	ProjMatrix.push_back(math::mat4(1.0f));
	ChangeProjMatrix.push_back(true);

	LastX.push_back(0.0f);
	LastY.push_back(0.0f);

	FirstTime.push_back(true);

	return NumCameras++;
}

/// @copydoc
void useCamera(const CameraId id)
{
	CLZ_ASSERT(id < NumCameras, "Invalid CameraId");

	if (LastActiveCamera == id)
		return;

	for (auto i = 0; i < NumCameras; i++)
	{
		FirstTime[i] = true;
	}

	LastActiveCamera = id;
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
	FirstTime[id] = true;
	ChangeProjMatrix[id] = true;
}

/// @copydoc
void updateCamera(const CameraId id)
{
	CLZ_ASSERT(id < NumCameras, "Invalid CameraId");
	CLZ_ASSERT(id == LastActiveCamera, "You forgot to use useCamera function");

	const math::vec2 cursorPos = window::getCursorPosition();
	const float scroll = window::getScrollOffset();
	processKeyBoardInput(id);
	processMouseInput(id, cursorPos.x, cursorPos.y);
	processMouseScroll(id, scroll);
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

/// @copydoc
void setCameraFirstTime(const CameraId Id)
{
	CLZ_ASSERT(Id < NumCameras, "Invalid CameraId");
	FirstTime[Id] = true;
}
} // namespace clz::renderer