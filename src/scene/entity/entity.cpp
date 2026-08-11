/**
 * @file entity.cpp
 * @author curl0z
 * @brief Entity subsystem implementation
 */

#include "../../../include/scene/entity/entity.hpp"
#include "../../../include/scene/entity/loader.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "entity/entitymanager.hpp"
#include "physics/physicscomponent.hpp"
#include "renderer/model/model.hpp"
#include "renderer/rendercomponent.hpp"

namespace clz::scene
{
	/**
	 * @brief Loads all entities from JSON file
	 * Iteratively loops over all entities inside the "entities" table inside JSON
	 * And retrieves the components registered under that entity index
	 * by calling that component's respective loader function.
	 * If editor is enabled, function will load create extra euler rotation and
	 * editor transform component for editor purposes.
	 * After loading, it will flag each subsystem that entities have been loaded.
	 *
	 * @param entityJson JSON's entity-table containing entity data
	 * @note It is necessary for each entity to have a name, if not it will be marked
	 * "Unnamed Entity"
	 */
	bool loadEntities(const nlohmann::json& entityJson)
	{
		for (auto& entityData : entityJson)
		{
			/// @brief Name is always retrieved first
			ecs::entity e;
			if (!entityData.contains("name"))
			{
				clz::log::warn("an entity is unnamed");
				e = ecs::createEntity("Unnamed Entity");
			}
			else
			{
				e = ecs::createEntity(entityData["name"]);
			}

			// Attach TransformComponent, should be present on every entity
			if (!entityData.contains("transform"))
			{
				clz::log::warn(
					"Entity: " + ecs::entityName[e] +
					"Does not have transform component"
					"Assigning it identity transform component"
				);
				ecs::addComponent<ecs::TransformComponent>(
					e,
					ecs::TransformComponent()
				);
#ifdef CLZ_ENABLE_EDITOR
				/// @brief If Editor is enabled,
				/// only then create these components
				ecs::addComponent<ecs::EditorTransformComponent>(
					e,
					ecs::EditorTransformComponent(ecs::TransformComponent())
				);
#endif
			}
			else
			{
				const auto transform =
					retrieveTransformComponent(entityData["transform"]);
				addComponent<ecs::TransformComponent>(e, transform);
#ifdef CLZ_ENABLE_EDITOR
				/// @brief If Editor is enabled,
				/// only then create these components
				ecs::addComponent<ecs::EditorTransformComponent>(
					e,
					ecs::EditorTransformComponent(transform)
				);
#endif
			}

			// Attach ModelComponent if present
			if (entityData.contains("model"))
			{
				ecs::addComponent<renderer::ModelComponent>(
					e,
					retrieveModelComponent(entityData["model"]["path"])
				);
			}
			// Attach Dir light Component if present
			if (entityData.contains("directionallight"))
			{
				const auto result = retrieveDirectionalLightComponent(
					entityData["directionallight"]
				);
				if (!result)
				{
					clz::log::warn(result.error());
					clz::log::warn("skipping this component");
				}
				else
				{
					ecs::addComponent<renderer::DirectionalLightComponent>(
						e,
						result.value()
					);
				}
			}
			// Attach point light component
			if (entityData.contains("pointlight"))
			{
				const auto result =
					retrievePointLightComponent(entityData["pointlight"]);
				if (!result)
				{
					clz::log::warn(result.error());
					clz::log::warn("skipping this component");
				}
				else
				{
					ecs::addComponent<renderer::PointLightComponent>(
						e,
						result.value()
					);
				}
			}

			// Attach physics Component if present
			if (entityData.contains("rigidbody"))
			{
				auto body = retrieveBodyComponent(entityData["rigidbody"], e);
				ecs::addComponent<physics::RigidBodyComponent>(e, body);
			}
		}

		// Entities loaded flag
		renderer::flagRenderComponentsLoaded();

		clz::log::info("Loaded entities");
		return true;
	}

	/// @brief Writes back all entity data inside the entity.json file
	/// @param sceneJson The main scene's JSON file
	void saveEntities(nlohmann::json& sceneJson)
	{
		sceneJson["entities"] = nlohmann::json::array();

		for (const auto& entity : ecs::entities)
		{
			nlohmann::json entityJson;
			entityJson["name"] = ecs::getEntityName(entity);

			/// --- 1. Transform components ---
			if (ecs::hasComponent<ecs::TransformComponent>(entity))
			{
				saveTransformComponent(
					ecs::getComponent<ecs::TransformComponent>(entity),
					entityJson["transform"]
				);
			}

			/// --- 2. Renderer components ---
			if (ecs::hasComponent<renderer::ModelComponent>(entity))
			{
				saveModelComponent(
					ecs::getComponent<renderer::ModelComponent>(entity),
					entityJson["model"]
				);
			}
			if (ecs::hasComponent<renderer::DirectionalLightComponent>(entity))
			{
				saveDirectionalLightComponent(
					ecs::getComponent<renderer::DirectionalLightComponent>(
						entity
					),
					entityJson["directionallight"]
				);
			}
			if (ecs::hasComponent<renderer::PointLightComponent>(entity))
			{
				savePointLightComponent(
					ecs::getComponent<renderer::PointLightComponent>(entity),
					entityJson["pointlight"]
				);
			}

			/// --- 3. Physics components ---
			if (ecs::hasComponent<physics::RigidBodyComponent>(entity))
			{
				saveRigidBodyComponent(
					ecs::getComponent<physics::RigidBodyComponent>(entity),
					entityJson["rigidbody"]
				);
			}
			sceneJson["entities"].push_back(entityJson);
		}
	}

} // namespace clz::scene