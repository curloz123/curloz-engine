#include "scene/entity/componentloader/loader.hpp"
#include "renderer/rendercomponent.hpp"

namespace clz::ecs
{
	ModelComponent retrieveModelComponent(const std::filesystem::path& path)
	{
		const ModelComponent modelComponent = renderer::createModelComponent(path);
		if (modelComponent.modelID == renderer::NULL_ASSET)
		{
			clz::log::error("Could not load model component");
		}

		return modelComponent;
	}

	void saveModelComponent(const ModelComponent& mc, nlohmann::json& componentData)
	{
		componentData["path"] = renderer::Asset::getModelName(mc.modelID);
	}
} // namespace clz::ecs
