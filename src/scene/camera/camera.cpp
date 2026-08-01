/**
 * @brief camera.cpp
 * @author curl0z
 * @brief Contains scene camera loading function
 * @warning not to be confused with renderer's camera.cpp
 */

#include "scene/camera/camera.hpp"
#include "core/logs.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/camera/cameradata.hpp"
#include "renderer/vk_types.hpp"

#ifdef CLZ_ENABLE_EDITOR
#include "include/offscreen/offscreentarget.hpp"
#include "include/sceneview.hpp"
#endif

namespace clz::scene
{
/// @copydoc
bool loadCameras(const nlohmann::json& cameraJson)
{
	/// --- 1. load cameras that are saved in json ---
	/// right now, only game camera is saved
	renderer::CameraDef gameCameraDef{};
	for (const auto& cam : cameraJson)
	{
		const std::string name = cam["name"].get<std::string>();
		if (name == "game")
		{
			gameCameraDef = {
				.fov = cam["fov"].get<float>(),
				.nearPlane = cam["near"].get<float>(),
				.farPlane = cam["far"].get<float>(),
				.maxVelocity = cam["maxvelocity"].get<float>(),
				.sensitivity = cam["sensitivity"].get<float>(),
				.acceleration = cam["acceleration"].get<float>(),
				.pitch = cam["pitch"].get<float>(),
				.yaw = cam["yaw"].get<float>(),
				.position = math::vec3(
					cam["position"][0].get<float>(),
					cam["position"][1].get<float>(),
					cam["position"][2].get<float>()
				),
				.localFront = math::vec3(
					cam["localfront"][0].get<float>(),
					cam["localfront"][1].get<float>(),
					cam["localfront"][2].get<float>()
				),
			};

			renderer::r_cameraId = renderer::createCamera(gameCameraDef);
			clz::log::info("loaded game camera");
		}
	}

#ifdef CLZ_ENABLE_EDITOR
	/// --- 2. Load editor camera ---

	/// Editor starts same as game camera
	renderer::CameraDef editorCameraDef = gameCameraDef;
	editorCameraDef.acceleration = 32768.0f;
	editor::mainViewportImage.cameraId = renderer::createCamera(editorCameraDef);

	/// --- 3. Load rigidbody shape editor camera ---
	renderer::CameraDef bodyCameraDef = {};
	bodyCameraDef.position = math::vec3(0.0f, 0.0f, 1.0f);
	bodyCameraDef.localFront = math::vec3(0.0f, 0.0f, -1.0f);
	bodyCameraDef.yaw = -90.0f;
	bodyCameraDef.pitch = 0.0f;
	editor::physicsBodyShapeImage.cameraId = renderer::createCamera(bodyCameraDef);

#endif

	clz::log::info("Loaded cameras");
	return true;
}

void saveCameras(nlohmann::json& sceneFile)
{
	nlohmann::json cam;

	/// only save game camera
	const auto id = renderer::r_cameraId;
	cam["maxvelocity"] = renderer::MaxVelocity[id];
	cam["sensitivity"] = renderer::Sensitivity[id];
	cam["acceleration"] = renderer::Acceleration[id];
	cam["pitch"] = renderer::Pitch[id];
	cam["yaw"] = renderer::Yaw[id];
	cam["fov"] = renderer::Fov[id];
	cam["near"] = renderer::Near[id];
	cam["far"] = renderer::Far[id];
	cam["position"][0] = renderer::Position[id].x;
	cam["position"][1] = renderer::Position[id].y;
	cam["position"][2] = renderer::Position[id].z;
	cam["localfront"][0] = renderer::LocalFront[id].x;
	cam["localfront"][1] = renderer::LocalFront[id].y;
	cam["localfront"][2] = renderer::LocalFront[id].z;
	cam["name"] = "game";

	/// create the camera array and push the camera data
	sceneFile["camera"] = nlohmann::json::array();
	sceneFile["camera"].push_back(cam);
}
} // namespace clz::scene