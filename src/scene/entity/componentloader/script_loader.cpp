#include "scene/entity/loader.hpp"
#include "script/script_components.hpp"

namespace clz::scene
{
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

	void saveSensorScriptComponent(
		const script::SensorScriptComponent& sensorScriptComponent,
		nlohmann::json& sensorScriptsArray
	)
	{
		std::vector<std::string> scriptPaths;
		sensorScriptComponent.getScriptPaths(scriptPaths);
		for (const auto& scriptPath : scriptPaths)
		{
			sensorScriptsArray.push_back(scriptPath);	
		}
	}


}
