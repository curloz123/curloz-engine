#pragma once

#include "cameradata.hpp"
#include "core/time.hpp"
#include "math/angle.hpp"
#include "window/inputmanager.hpp"
#include <fstream>

#define JSON_HAS_STATIC_RTTI 0
#define JSON_NOEXCEPTION
#include <nlohmann/json.hpp>
#include "core/enginestate.hpp"

namespace clz::renderer::camera
{
	/// @brief Loads a named camera entry from config/scene.json into slot @p id.
	/// @return true on success, false if the file or entry could not be found/parsed.
	inline bool loadCamera(const std::string& name, const CameraID id)
	{

		if (const std::filesystem::path file = "config/scene.json";
			!std::filesystem::exists(file))
		{
			clz::log::error("No file named 'scene.json' found in config directory");
			return false;
		}

		std::ifstream file("config/scene.json");
		const nlohmann::json data = nlohmann::json::parse(file,
					nullptr, false);
		if (data.is_discarded())
		{
			clz::log::error("Failed to load camera in json");
		}

		for (const auto& cam : data["camera"])
		{
			if (cam["name"] != name)
				continue;

			MaxVelocity[id]  = cam["maxvelocity"].get<float>();
			Sensitivity[id]  = cam["sensitivity"].get<float>();
			Acceleration[id] = cam["acceleration"].get<float>();
			Pitch[id]        = cam["pitch"].get<float>();
			Yaw[id]          = cam["yaw"].get<float>();
			Fov[id]           = cam["fov"].get<float>();

			const auto& pos = cam["position"];
			Position[id] = math::vec3(pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>());

			const auto& front = cam["localfront"];
			localFront[id] = math::vec3(front[0].get<float>(), front[1].get<float>(), front[2].get<float>());

			const auto& right = cam["right"];
			Right[id] = math::vec3(right[0].get<float>(), right[1].get<float>(), right[2].get<float>());

			return true;
		}

		clz::log::error("Camera entry not found in json: " + name);
		return false;
	}

	/// @brief Applies WASD movement with acceleration/friction to camera @p id.
	inline void processKeyBoardInput(const CameraID id)
	{
		auto dir = math::vec3(0.0f, 0.0f, 0.0f);
		const float dt = time::getDeltaTime();

		if (window::isPressed(input::Key::W)) dir += localFront[id];
		if (window::isPressed(input::Key::S)) dir -= localFront[id];
		if (window::isPressed(input::Key::A)) dir -= Right[id];
		if (window::isPressed(input::Key::D)) dir += Right[id];

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

	/// @brief Recomputes forward/right basis vectors for camera @p id from pitch/yaw.
	inline void updateCameraVectors(const CameraID id)
	{
		localFront[id].x = std::cos(math::radians(Yaw[id])) * std::cos(math::radians(Pitch[id]));
		localFront[id].y = std::sin(math::radians(Pitch[id]));
		localFront[id].z = std::sin(math::radians(Yaw[id])) * std::cos(math::radians(Pitch[id]));
		localFront[id] = math::normalize(localFront[id]);

		Right[id] = math::normalize(math::cross(localFront[id], WorldUp));
	}

	/// @brief Updates pitch/yaw for camera @p id from raw cursor position.
	inline void processMouseInput(const CameraID id, const float xPos, const float yPos)
	{
		if (FirstTime[id])
		{
			LastX[id] = xPos;
			LastY[id] = yPos;
			FirstTime[id] = false;
		}

		const float xOff = xPos - LastX[id];
		const float yOff = -(yPos - LastY[id]);
		LastX[id] = xPos;
		LastY[id] = yPos;

		if (state::g_engineState == state::EngineState::Sandbox && !window::isPressed(input::Key::F))
			return;

		Yaw[id]   += xOff * Sensitivity[id];
		Pitch[id] += yOff * Sensitivity[id];
		Pitch[id] = std::clamp(Pitch[id], -89.0f, 89.0f);

		updateCameraVectors(id);
	}

	/// @brief Adjusts FOV for camera @p id from scroll input, clamped to [1, 89] degrees.
	inline void processMouseScroll(const CameraID id, const float yOffset)
	{
		Fov[id] -= yOffset;
		if (Fov[id] < 1.0f)
			Fov[id] = 1.0f;
		if (Fov[id] > 89.0f)
			Fov[id] = 89.0f;
	}

}