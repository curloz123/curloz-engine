#include "scene/assets/models.hpp"
#include "renderer/model/model.hpp"

namespace clz::scene
{
	void loadAllModels(const nlohmann::json& modelPathArray)
	{
		std::vector<std::filesystem::path> modelPaths;
		modelPaths.reserve(modelPathArray.size());

		for (const auto& modelPath : modelPathArray)
		{
			if (!modelPath.is_null())
				modelPaths.push_back(modelPath);
			else
				modelPaths.push_back(
					renderer::ModelManagerClass::NULL_MODEL_PATH
				);
		}

		renderer::ModelManager.loadBatch(modelPaths);
	}

	void saveBackModelPaths(nlohmann::json& modelPathArray)
	{
		const auto& modelPaths = renderer::ModelManager.getModelPaths();
		for (const auto& modelPath : modelPaths)
		{
			modelPathArray.push_back(modelPath);
		}
	}
}
