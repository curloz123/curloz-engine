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

#include "ecs/scene.hpp"
#include "filesystem"
#include "core/logs.hpp"
#include "ecs/components.hpp"
#include "ecs/entitymanager.hpp"
#include "renderer/rendercomponent.hpp"
#include "math/quateulerconv.hpp"
#include <fstream>


namespace clz::ecs
{
	TransformComponent retrieveTransformComponent(const nlohmann::json& componentData);
	ModelComponent retrieveModelComponent(const std::filesystem::path& path);
}

namespace clz::ecs
{
	bool loadEntities()
	{
		clz::log::debug("Loading entities");

		// Read scene file path from config
		const std::filesystem::path entityFile = "config/scene.json";
		if (!std::filesystem::exists(entityFile))
		{
			clz::log::error("No file named 'scene.json' found in config directory");
			return false;
		}


		// Parse JSON
		std::ifstream file(entityFile);
		if (!file.is_open())
		{
			clz::log::error("Could not open entity file: " + std::string(entityFile));
			return false;
		}
		const nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
		if (json.is_discarded())
		{
			clz::log::error("Could not parse JSON file: " + std::string(entityFile));
			return false;
		}

		// Create entities from JSON
		for (auto& entityData : json["entities"])
		{
			entity e;
			if (!entityData.contains("name"))
			{
				clz::log::warn("an entity is unnamed");
				e = createEntity("Unnamed Entity");
			}
			else
			{
				e = createEntity(entityData["name"]);
			}


			// Attach TransformComponent if present
			if (entityData.contains("transform"))
			{
				addComponent<TransformComponent>(e, retrieveTransformComponent(entityData["transform"]));
			#ifdef CLZ_ENABLE_SANDBOX
				addComponent<EulerRotationComponent>(e, {math::quatToEulerXYZ(
					getComponent<TransformComponent>(e).rotation)});
			#endif
			}

			// Attach ModelComponent if present
			if (entityData.contains("model"))
			{
				if (!hasComponent<TransformComponent>(e))
				{
					clz::log::warn("Entity: " + entityName[e] +
						"Does not have transform component\nAssigning it identity transform component");

					addComponent<TransformComponent>(e, TransformComponent());
				}

				addComponent<ModelComponent>(
					e, retrieveModelComponent(entityData["model"]["path"]));
			}
		}

		// Entities loaded flag
		renderer::flagRenderComponentsLoaded();

		clz::log::info("Loaded all entities");
		return true;
	}


	TransformComponent retrieveTransformComponent(const nlohmann::basic_json<>& componentData)
	{
		const clz::math::quat q(
			componentData["rotation"][0],
			componentData["rotation"][1],
			componentData["rotation"][2],
			componentData["rotation"][3]);
		const clz::math::vec3 t(
			componentData["position"][0],
			componentData["position"][1],
			componentData["position"][2]);
		const clz::math::vec3 s(
			componentData["scale"][0],
			componentData["scale"][1],
			componentData["scale"][2]);

		return {q, t, s};
	}

	ModelComponent retrieveModelComponent(const std::filesystem::path& path)
	{
		const ModelComponent modelComponent = renderer::createModelComponent(path);
		if (modelComponent.modelID == renderer::NULL_ASSET)
		{
			clz::log::error("Could not load model component");
		}

		return modelComponent;
	}

} // namespace clz::ecs
