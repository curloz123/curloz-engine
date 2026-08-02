/**
 * @file modelloader.cpp
 * @author curl0z
 * @brief Loads model component of an entity
 */

#include "scene/entity/loader.hpp"
#include "renderer/rendercomponent.hpp"

namespace clz::scene
{
/// @brief Retrieves Model component of any entity.
/// @param path of entity's model
/// @return ModelComponent of given entity
/// @note will return NULL_ASSET if renderer could not load model
/// and log an error
renderer::ModelComponent retrieveModelComponent(const std::filesystem::path& path)
{
	const renderer::ModelComponent modelComponent = renderer::createModelComponent(path);
	if (modelComponent.modelId == renderer::NULL_MODEL)
	{
		clz::log::error("Could not load model component");
	}

	return modelComponent;
}

/// @brief Saves model component of any entity.
/// @param mc modelComponent of given entity
/// @param componentData JSON index of entity
void saveModelComponent(const renderer::ModelComponent& mc, nlohmann::json& componentData)
{
	componentData["path"] = renderer::getModelPath(mc.modelId);
}
} // namespace clz::scene
