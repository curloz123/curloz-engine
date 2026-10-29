#pragma once

#include <nlohmann/json.hpp>

namespace clz::scene
{
	void loadAllModels(const nlohmann::json& modelPathArray);

	void saveBackModelPaths(nlohmann::json& modelPathArray);
}
