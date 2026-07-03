/**
 * @brief camera.cpp
 * @author curl0z
 * @brief Contains implementation of scene camera
 * @warning not to be confused with renderer's camera
 */

#include "scene/camera/camera.hpp"
#include "core/logs.hpp"
#include "renderer/camera/camera.hpp"

using namespace clz::renderer::camera;

namespace clz::scene
{
	bool loadCameras(const nlohmann::json& cameraJson)
	{
		for (const auto& cam : cameraJson)
		{
			const auto id = static_cast<CameraID>(cam["id"]);

			MaxVelocity[id] = cam["maxvelocity"].get<float>();
			Sensitivity[id] = cam["sensitivity"].get<float>();
			Acceleration[id] = cam["acceleration"].get<float>();
			Pitch[id] = cam["pitch"].get<float>();
			Yaw[id] = cam["yaw"].get<float>();
			Fov[id] = cam["fov"].get<float>();
			Near[id] = cam["near"].get<float>();
			Far[id] = cam["far"].get<float>();

			const auto& pos = cam["position"];
			Position[id] = math::vec3(pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>());

			const auto& front = cam["localfront"];
			auto jFront = math::vec3(front[0].get<float>(), front[1].get<float>(), front[2].get<float>());
			localFront[id] = math::normalize(jFront);

			localRight[id] = math::normalize(math::cross(localFront[id], WorldUp));
		}

		clz::log::info("Loaded cameras");
		return true;
	}

	void saveCameras(nlohmann::json& sceneFile)
	{
		sceneFile["camera"] = nlohmann::json::array();

		for (int id = 0; id < NumCameras; ++id)
		{
			nlohmann::json cam;
			cam["position"][0] = Position[id].x;
			cam["position"][1] = Position[id].y;
			cam["position"][2] = Position[id].z;
			cam["localfront"][0] = localFront[id].x;
			cam["localfront"][1] = localFront[id].y;
			cam["localfront"][2] = localFront[id].z;
			cam["id"] = id;
			cam["maxvelocity"] = MaxVelocity[id];
			cam["sensitivity"] = Sensitivity[id];
			cam["acceleration"] = Acceleration[id];
			cam["pitch"] = Pitch[id];
			cam["yaw"] = Yaw[id];
			cam["fov"] = Fov[id];
			cam["near"] = Near[id];
			cam["far"] = Far[id];

			sceneFile["camera"].push_back(cam);
		}
	}
} // namespace clz::scene