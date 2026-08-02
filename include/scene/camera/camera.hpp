/**
 * file camera.hpp
 * @author curl0z
 * @brief Defines scene camera's loading and saving utility.
 * @warning not to be confused with renderer's camera
 */

#pragma once

#include "nlohmann/json.hpp"

namespace clz::scene
{
bool loadCameras(const nlohmann::json& cameraJson);
void saveCameras(nlohmann::json& sceneFile);
} // namespace clz::scene