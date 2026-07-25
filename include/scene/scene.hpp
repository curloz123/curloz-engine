/**
 * @file scene.hpp
 * @author curl0z
 * @brief Scene loading. Parses entity JSON and populates the ECS registry.
 * @note config::init() must be called before loadEntities().
 * @warning Scene must always be the last subsystem to be initiated
 * as it initializes entity data for them, they must be ready.
 */

#pragma once

#include <nlohmann/json.hpp>

namespace clz::scene
{
	/// @brief nlohmann's JSON Handle
	inline nlohmann::json sceneFile;

	/// @brief Loads Scene from the JSON scene file.
	/// @return true on success, false on failure and prints the error
	bool loadScene();

	/// @brief Saves scene back to JSON
	void saveScene();

} // namespace clz::scene