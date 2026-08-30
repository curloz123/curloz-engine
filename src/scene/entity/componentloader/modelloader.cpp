/**
 * @file modelloader.cpp
 * @author curl0z
 * @brief Loads model component of an entity
 */

#include "core/logs.hpp"
#include "renderer/rendercomponent.hpp"
#include "scene/entity/loader.hpp"

namespace clz::scene
{
	/// @copydoc retrieveModelComponent
	renderer::ModelComponent retrieveModelComponent(
			const std::filesystem::path& path,
			std::string_view entityName)
	{
		const renderer::ModelComponent modelComponent =
			renderer::createModelComponent(path);
		if (modelComponent.modelId == renderer::NULL_MODEL)
		{
			clz::log::error(
				"Could not load model component for entity: " + 
				std::string(entityName)
			);
		}

		return modelComponent;
	}

	/// @copydoc saveModelComponent
	void saveModelComponent(
			const renderer::ModelComponent& mc,
			nlohmann::json& componentData,
			std::string_view entityName)
	{
		componentData["path"] = renderer::getModelPath(mc.modelId);

		clz::log::info("Saved model component for entity: " + std::string(entityName));
	}
} // namespace clz::scene
