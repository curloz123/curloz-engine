/**
 * @file script_loader.cpp
 * @author curl0z
 * @brief Implementation of all script components loaders
 */

#include "scene/entity/loader.hpp"
#include "script/script_components.hpp"

namespace clz::scene
{
	/// @copydoc retrieveSensorScriptComponent
	script::SensorScriptComponent retrieveSensorScriptComponent(
		const nlohmann::json& scriptsPathArray
	)
	{
		script::SensorScriptComponent sensorScriptComponent;
		for (const auto& scriptPath : scriptsPathArray)
		{
			std::filesystem::path path(scriptPath);
			sensorScriptComponent.loadScript(path);
		}

		return sensorScriptComponent;
	}

	/// @copydoc saveSensorScriptComponent
	void saveSensorScriptComponent(
		const script::SensorScriptComponent& sensorScriptComponent,
		nlohmann::json& sensorScriptsArray
	)
	{
		std::vector<std::string> scriptPaths;
		sensorScriptComponent.getScriptPaths(scriptPaths);
		clz::log::debug("Script saving time size: " + std::to_string(scriptPaths.size()));
		for (const auto& scriptPath : scriptPaths)
		{
			sensorScriptsArray.push_back(scriptPath);	
		}
	}

	/// @copydoc retrieveCollisionScriptComponent
	script::CollisionScriptComponent retrieveCollisionScriptComponent(
		const nlohmann::json& scriptsPathArray
	)
	{
		script::CollisionScriptComponent collisionScriptComponent;
		for (const auto& scriptPath : scriptsPathArray)
		{
			std::filesystem::path path(scriptPath);
			collisionScriptComponent.loadScript(path);
		}

		return collisionScriptComponent;
	}

	/// @copydoc saveCollisionScriptComponent
	void saveCollisionScriptComponent(
		const script::CollisionScriptComponent& collisionScriptComponent,
		nlohmann::json& collisionScriptsArray
	)
	{
		std::vector<std::string> scriptPaths;
		collisionScriptComponent.getScriptPaths(scriptPaths);
		clz::log::debug("Script saving time size: " + std::to_string(scriptPaths.size()));
		for (const auto& scriptPath : scriptPaths)
		{
			collisionScriptsArray.push_back(scriptPath);
		}
	}

}
