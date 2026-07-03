/**
 * @file entity.cpp
 * @author curl0z
 * @brief Entity subsystem implmentation
 */

#include "scene/entity/entity.hpp"
#include "math/quateulerconv.hpp"
#include "renderer/assets/modeldata.hpp"
#include "renderer/rendercomponent.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"
#include "scene/entity/entitymanager.hpp"

namespace clz::ecs
{
	/**
	 * @brief Retrieves transform component of any entity.
	 * @param componentData json index of entity
	 * @return TransformComponent of given entity
	 */
	TransformComponent retrieveTransformComponent(const nlohmann::json& componentData);
	/**
	 * @brief Retrieves Model component of any entity.
	 * @param path of entity's model
	 * @return ModelComponent of given entity
	 */
	ModelComponent retrieveModelComponent(const std::filesystem::path& path);

	/**
	 * @brief Saves transform component of any entity.
	 * @param tc TransformComponent of given entity
	 * @param componentData json index of entity
	 */
	void saveTransformComponent(TransformComponent tc, nlohmann::json& componentData);

	/**
	 * @brief Saves model component of any entity.
	 * @param mc modelComponent of given entity
	 * @param componentData json index of entity
	 */
	void saveModelComponent(ModelComponent mc, nlohmann::json& componentData);
} // namespace clz::ecs

namespace clz::ecs
{
	bool loadEntities(const nlohmann::json& entityJson)
	{
		for (auto& entityData : entityJson)
		{
			ecs::entity e;
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
				addComponent<EulerRotationComponent>(e, {math::quatToEulerXYZ(getComponent<TransformComponent>(e).rotation)});
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

				addComponent<ModelComponent>(e, retrieveModelComponent(entityData["model"]["path"]));
			}
		}

		// Entities loaded flag
		renderer::flagRenderComponentsLoaded();

		clz::log::info("Loaded entities");
		return true;
	}

	void saveEntities(nlohmann::json& sceneJson)
	{
		sceneJson["entities"] = nlohmann::json::array();

		for (const auto& entity : entities)
		{
			nlohmann::json entityJson;
			entityJson["name"] = getEntityName(entity);

			if (hasComponent<TransformComponent>(entity))
			{
				saveTransformComponent(getComponent<TransformComponent>(entity), entityJson["transform"]);
			}

			if (hasComponent<ModelComponent>(entity))
			{
				saveModelComponent(getComponent<ModelComponent>(entity), entityJson["model"]);
			}

			sceneJson["entities"].push_back(entityJson);
		}
	}

	void destroyEntities()
	{
		clearEntities();
		clz::log::info("Destroyed Entities");
	}
} // namespace clz::ecs

namespace clz::ecs
{
	TransformComponent retrieveTransformComponent(const nlohmann::basic_json<>& componentData)
	{
		const clz::math::quat q(componentData["rotation"][0], componentData["rotation"][1], componentData["rotation"][2],
					componentData["rotation"][3]);
		const clz::math::vec3 t(componentData["position"][0], componentData["position"][1], componentData["position"][2]);
		const clz::math::vec3 s(componentData["scale"][0], componentData["scale"][1], componentData["scale"][2]);

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

	void saveTransformComponent(TransformComponent tc, nlohmann::json& componentData)
	{
		componentData["rotation"][0] = tc.rotation.w;
		componentData["rotation"][1] = tc.rotation.x;
		componentData["rotation"][2] = tc.rotation.y;
		componentData["rotation"][3] = tc.rotation.z;
		componentData["position"][0] = tc.position.x;
		componentData["position"][1] = tc.position.y;
		componentData["position"][2] = tc.position.z;
		componentData["scale"][0] = tc.scale.x;
		componentData["scale"][1] = tc.scale.y;
		componentData["scale"][2] = tc.scale.z;
	}

	void saveModelComponent(ModelComponent mc, nlohmann::json& componentData)
	{
		componentData["path"] = renderer::Asset::getModelName(mc.modelID);
	}
} // namespace clz::ecs
