/**
 * @file camera.hpp
 * @author curl0z
 * @brief Free-floating offscreen preview camera used by editor offscreen
 * targets (physics-body shape editor, model viewer, etc). Since only one
 * offscreen target is ever shown at a time, this is deliberately global
 * state rather than a per-instance object.
 * Whenever a new instance is shown, it is recommended to run reset Camera.
 */
#pragma once

#include "core/time.hpp"
#include "math/mat4x4.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/angle.hpp"
#include "math/worldtransform.hpp"
#include "window/inputmanager.hpp"
#include "window/mouse.hpp"
#include <algorithm>

namespace clz::editor::camera
{
	/// @brief Shared world-space up reference used for basis derivation.
	const auto WorldUp = math::vec3(0.0f, 1.0f, 0.0f);

	constexpr float Near = 0.1f;
	constexpr float Far = 100.0f;

	/// @brief Camera world-space position.
	inline math::vec3 Position = math::vec3(0.0f, 0.0f, 3.0f);

	/// @brief Camera forward vector, derived from Yaw/Pitch.
	inline math::vec3 localFront = math::vec3(0.0f, 0.0f, -1.0f);

	/// @brief Camera right vector, derived from localFront x WorldUp.
	inline math::vec3 localRight = math::vec3(1.0f, 0.0f, 0.0f);

	inline float Pitch = 0.0f;
	inline float Yaw = -90.0f;
	constexpr float Sensitivity = 0.4f;

	/// @brief Current movement velocity, with acceleration/friction applied per frame.
	inline math::vec3 Velocity = math::vec3(0.0f, 0.0f, 0.0f);
	constexpr float MaxVelocity = 5.0f;
	constexpr float Acceleration = 2.0f;

	inline float Fov = 45.0f;

	/// @brief Last recorded cursor position, for mouse delta calculation.
	inline math::vec2 LastMousePos = math::vec2(0.0f, 0.0f);

	/// @brief Avoids a mouse-delta snap on first use.
	inline bool FirstTime = true;
}

namespace clz::editor::camera
{
	/// @brief Processes WASD movement and right-click-drag look/scroll-zoom for one frame.
	inline void update()
	{
		// Process Event
		auto dir = math::vec3(0.0f, 0.0f, 0.0f);
		const float dt = time::getDeltaTime();

		if (window::isKeyPressed(input::Key::W))
			dir += localFront;
		if (window::isKeyPressed(input::Key::S))
			dir -= localFront;
		if (window::isKeyPressed(input::Key::A))
			dir -= localRight;
		if (window::isKeyPressed(input::Key::D))
			dir += localRight;
		if (window::isKeyPressed(input::Key::Space))
			dir += WorldUp;
		if (window::isKeyPressed(input::Key::LeftAlt))
			dir -= WorldUp;

		if (math::getLengthSquared(dir) > 0.0f)
		{
			dir = math::normalize(dir);
			Velocity += dir * Acceleration * dt;

			if (math::getLength(Velocity) >= MaxVelocity)
			{
				Velocity = math::normalize(Velocity) * MaxVelocity;
			}
		}
		else
		{
			if (const float speed = math::getLength(Velocity); speed > 0.0f)
			{
				const float drop = Acceleration * dt;
				const float newSpeed = std::max(speed - drop, 0.0f);
				Velocity = Velocity * (newSpeed / speed);
			}
		}

		Position += Velocity * dt;
		Position.x = std::clamp(Position.x, -25.0f, 25.0f);
		Position.y = std::clamp(Position.y, -25.0f, 25.0f);


		// Process Mouse Movement

		const auto mousePos = window::getCursorPosition();
		if (FirstTime)
		{
			LastMousePos.x = mousePos.x;
			LastMousePos.y = mousePos.y;
			FirstTime = false;
		}

		bool ProcessMouseInput = true;
		if (!window::isMousePressed(input::Mouse::MouseRight))
		{
			LastMousePos.x = mousePos.x;
			LastMousePos.y = mousePos.y;
			ProcessMouseInput = false;
		}
		if (ProcessMouseInput)
		{
			const float xOff = mousePos.x - LastMousePos.x;
			const float yOff = -(mousePos.y - LastMousePos.y);
			LastMousePos.x = mousePos.x;
			LastMousePos.y = mousePos.y;

			Yaw += xOff * Sensitivity;
			Pitch += yOff * Sensitivity;
			Pitch = std::clamp(Pitch, -89.0f, 89.0f);


			localFront.x = std::cos(math::radians(Yaw)) * std::cos(math::radians(Pitch));
			localFront.y = std::sin(math::radians(Pitch));
			localFront.z = std::sin(math::radians(Yaw)) * std::cos(math::radians(Pitch));
			localFront = math::normalize(localFront);

			localRight = math::normalize(math::cross(localFront, WorldUp));
		}

		const float mouseScroll = window::getScrollOffset();
		bool ProcessMouseScroll = true;
		if (mouseScroll == 0.0f)
		{
			ProcessMouseScroll = false;
		}
		if (ProcessMouseScroll)
		{
			Fov -= mouseScroll;
			if (Fov < 1.0f)
			{
				Fov = 1.0f;
			}
			if (Fov > 89.0f)
			{
				Fov = 89.0f;
			}
		}

	}

	/// @brief Returns the current view matrix, looking from Position toward Position + localFront.
	inline math::mat4 getViewMatrix()
	{
		return makeViewMatrix(Position, Position + localFront, WorldUp);
	}

	/// @brief Returns the current perspective projection matrix for the given aspect ratio.
	inline math::mat4 getProjectionMatrix(const float AspectRatio)
	{
		return math::makePerspectiveMatrix(
			Far, Near, AspectRatio, math::radians(Fov));
	}

	/// @brief Resets the camera to its default position/orientation/velocity.
	/// Called when opening a new offscreen preview target.
	inline void resetCamera()
	{
		Position = math::vec3(0.0f, 0.0f, 3.0f);
		Velocity = math::vec3(0.0f, 0.0f, 0.0f);
		localFront = math::vec3(0.0f, 0.0f, -1.0f);
		localRight = math::vec3(1.0f, 0.0f, 0.0f);
		Yaw = -90.0f;
		Pitch = 0.0f;
		FirstTime = true;
	}
}