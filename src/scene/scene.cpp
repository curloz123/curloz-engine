/**
 * @file scene.cpp
 * @author curl0z
 * @brief Scene loading implementation.
 *
 * Parses the entity JSON file and populates the EnTT registry
 * with entities and their components.
 */

#define JSON_HAS_STATIC_RTTI 0
#define JSON_NOEXCEPTION
#include <nlohmann/json.hpp>

#include "scene/entity/entity.hpp"
#include "core/logs.hpp"
#include "scene/camera/camera.hpp"
#include "scene/scene.hpp"
#include <fstream>

namespace clz::scene
{
	constexpr auto entityFile = "config/scene.json";
}
namespace clz::scene
{
	/// @copydoc
	bool loadScene()
	{
		// Parse Scene File
		std::ifstream file(entityFile);
		if (!file.is_open())
		{
			clz::log::error("Could not open entity file: " + std::string(entityFile));
			return false;
		}
		sceneFile = nlohmann::json::parse(file, nullptr, false);
		if (sceneFile.is_discarded())
		{
			clz::log::error("Could not parse JSON file: " + std::string(entityFile));
			return false;
		}

		// Load cameras
		if (!sceneFile.contains("camera"))
		{
			clz::log::error("Scene file does not have camera entry");
			return false;
		}
		if (!loadCameras(sceneFile["camera"]))
		{
			clz::log::error("Could not load cameras");
			return false;
		}

		// Load entities
		if (!sceneFile.contains("entities"))
		{
			clz::log::error("Scene file does not have entities entry");
			return false;
		}
		if (!loadEntities(sceneFile["entities"]))
		{
			clz::log::error("Could not load entities");
			return false;
		}

		clz::log::info("Loaded scene");
		return true;
	}

	/// @copydoc
	void saveScene()
	{
		// Clear file, we're gonna write back to it.
		sceneFile.clear();

		// Save camera and entities
		saveCameras(sceneFile);
		saveEntities(sceneFile);

		// Write back
		std::ofstream out("config/scene.json");
		out << sceneFile.dump(4);

		clz::log::info("Saved scene");
	}

	/// @copydoc
	void closeScene()
	{
		saveScene();
	}
} // namespace clz::scene