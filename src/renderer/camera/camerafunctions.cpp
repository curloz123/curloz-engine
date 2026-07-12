/**
 * @file camerafunctions.cpp
 * @author curl0z
 * @brief Camera functions implementation file
 */

#define JSON_HAS_STATIC_RTTI 0
#define JSON_NOEXCEPTION
#include "renderer/camera/camerafunctions.hpp"
#include "core/enginestate.hpp"
#include "core/logs.hpp"
#include "core/time.hpp"
#include "math/angle.hpp"
#include "window/inputmanager.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

namespace clz::renderer::camera
{
	bool loadCamera(const std::string& name, const CameraID id)
	{
		if (const std::filesystem::path file = "config/scene.json"; !std::filesystem::exists(file))
		{
			clz::log::error("No file named 'scene.json' found in config directory");
			return false;
		}

		std::ifstream file("config/scene.json");
		const nlohmann::json data = nlohmann::json::parse(file, nullptr, false);
		if (data.is_discarded())
		{
			clz::log::error("Failed to load camera in json");
		}

		for (const auto& cam : data["camera"])
		{
			if (cam["name"] != name)
				continue;

			MaxVelocity[id] = cam["maxvelocity"].get<float>();
			Sensitivity[id] = cam["sensitivity"].get<float>();
			Acceleration[id] = cam["acceleration"].get<float>();
			Pitch[id] = cam["pitch"].get<float>();
			Yaw[id] = cam["yaw"].get<float>();
			Fov[id] = cam["fov"].get<float>();

			const auto& pos = cam["position"];
			Position[id] = math::vec3(pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>());

			const auto& front = cam["localfront"];
			auto jFront = math::vec3(front[0].get<float>(), front[1].get<float>(), front[2].get<float>());
			localFront[id] = math::normalize(jFront);

			localRight[id] = math::cross(localFront[id], WorldUp);

			return true;
		}

		clz::log::error("Camera entry not found in json: " + name);
		return false;
	}

	void processKeyBoardInput(const CameraID id)
	{
		auto dir = math::vec3(0.0f, 0.0f, 0.0f);
		const float dt = time::getDeltaTime();

		if (window::isKeyPressed(input::Key::W))
			dir += localFront[id];
		if (window::isKeyPressed(input::Key::S))
			dir -= localFront[id];
		if (window::isKeyPressed(input::Key::A))
			dir -= localRight[id];
		if (window::isKeyPressed(input::Key::D))
			dir += localRight[id];
		if (window::isKeyPressed(input::Key::Space))
			dir += WorldUp;
		// if (window::isKeyPressed(input::Key::LeftShift))
		// dir -= WorldUp;

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

	void updateCameraVectors(const CameraID id)
	{
		localFront[id].x = std::cos(math::radians(Yaw[id])) * std::cos(math::radians(Pitch[id]));
		localFront[id].y = std::sin(math::radians(Pitch[id]));
		localFront[id].z = std::sin(math::radians(Yaw[id])) * std::cos(math::radians(Pitch[id]));
		localFront[id] = math::normalize(localFront[id]);

		localRight[id] = math::normalize(math::cross(localFront[id], WorldUp));
	}

	void processMouseInput(const CameraID id, const float xPos, const float yPos)
	{
		if (FirstTime[id])
		{
			LastX[id] = xPos;
			LastY[id] = yPos;
			FirstTime[id] = false;
		}

		if (state::g_engineState == state::EngineState::Editor && !window::isMousePressed(input::Mouse::MouseRight))
		{
			LastX[id] = xPos;
			LastY[id] = yPos;
			return;
		}

		const float xOff = xPos - LastX[id];
		const float yOff = -(yPos - LastY[id]);

		LastX[id] = xPos;
		LastY[id] = yPos;

		Yaw[id] += xOff * Sensitivity[id];
		Pitch[id] += yOff * Sensitivity[id];
		Pitch[id] = std::clamp(Pitch[id], -89.0f, 89.0f);

		updateCameraVectors(id);
	}

	void processMouseScroll(const CameraID id, const float yOffset)
	{
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
		FovChanged[id] = true;
	}
} // namespace clz::renderer::camera