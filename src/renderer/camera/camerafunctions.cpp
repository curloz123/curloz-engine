/**
 * @file camerafunctions.cpp
 * @author curl0z
 * @brief Camera functions implementation file
 */

#define JSON_HAS_STATIC_RTTI 0
#define JSON_NOEXCEPTION
#include "renderer/camera/camerafunctions.hpp"
#include "core/enginestate.hpp"
#include "core/time.hpp"
#include "math/angle.hpp"
#include "renderer/camera/camera.hpp"
#include "window/inputmanager.hpp"
#include <nlohmann/json.hpp>
#include "window/mouse.hpp"

namespace clz::renderer
{
	/// @copydoc
	void processKeyBoardInput(const CameraId id)
	{
		auto dir = math::vec3(0.0f, 0.0f, 0.0f);
		const float  dt = static_cast<float>(time::getDeltaTime());
		clz::log::debug("dt in camera: " + std::to_string(dt));

		if (window::isKeyPressed(input::Key::W))
			dir += LocalFront[id];
		if (window::isKeyPressed(input::Key::S))
			dir -= LocalFront[id];
		if (window::isKeyPressed(input::Key::A))
			dir -= LocalRight[id];
		if (window::isKeyPressed(input::Key::D))
			dir += LocalRight[id];
		if (window::isKeyPressed(input::Key::Space))
			dir += WorldUp;
		if (window::isKeyPressed(input::Key::LeftAlt))
			dir -= WorldUp;

		if (math::getLengthSquared(dir) > 0.0f)
		{
			dir = math::normalize(dir);
			Velocity[id] += dir * Acceleration[id] * dt;

			if (math::getLength(Velocity[id]) >= MaxVelocity[id])
			{
				Velocity[id] = math::normalize(Velocity[id]) * MaxVelocity[id];
			}
		}
		else
		{
			if (const float speed = math::getLength(Velocity[id]); speed > 0.0f)
			{
				const float drop = Acceleration[id] * dt;
				const float newSpeed = std::max(speed - drop, 0.0f);
				Velocity[id] = Velocity[id] * (newSpeed / speed);
			}
		}

		Position[id] += Velocity[id] * dt;
	}

	/// @copydoc
	void processMouseInput(const CameraId id)
	{
		const math::vec2 offset = window::getCursorOffset();
		Yaw[id] += offset.x * Sensitivity[id];
		Pitch[id] += -1.0f * offset.y * Sensitivity[id];
		Pitch[id] = std::clamp(Pitch[id], -89.0f, 89.0f);

		updateCameraVectors(id);
	}

	/// @copydoc
	void processMouseScroll(const CameraId id)
	{
		const float yOffset = window::getScrollOffset();

		if (yOffset == 0.0f)
		{
			return;
		}

		Fov[id] -= yOffset;
		if (Fov[id] < 1.0f)
		{
			Fov[id] = 1.0f;
		}
		if (Fov[id] > 89.0f)
		{
			Fov[id] = 89.0f;
		}

		ChangeProjMatrix[id] = true;
	}

	/// @copydoc
	void updateCameraVectors(const CameraId id)
	{
		LocalFront[id].x =
			std::cos(math::radians(Yaw[id])) * std::cos(math::radians(Pitch[id]));
		LocalFront[id].y = std::sin(math::radians(Pitch[id]));
		LocalFront[id].z =
			std::sin(math::radians(Yaw[id])) * std::cos(math::radians(Pitch[id]));
		LocalFront[id] = math::normalize(LocalFront[id]);

		LocalRight[id] = math::normalize(math::cross(LocalFront[id], WorldUp));
	}

} // namespace clz::renderer
