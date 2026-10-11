/**
 * @file entity.cpp
 * @author curl0z
 * @brief Entity subsystem implementation
 */

#include "scene/entity/entity.hpp"
#include "audio/audio_components.hpp"
#include "scene/entity/loader.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "entity/entitymanager.hpp"
#include "physics/physicscomponent.hpp"
#include "renderer/model/model.hpp"
#include "renderer/rendercomponent.hpp"
#include "script/cross_system_flags.hpp"
#include "script/script_components.hpp"

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
			std::string entityName = "Unnamed Entity";
			if (!entityData.contains("name"))
				clz::log::warn("an entity is unnamed");
			else
				entityName = entityData["name"];

			e = ecs::createEntity(entityName);


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
				const auto transform = retrieveTransformComponent(
								entityData["transform"], entityName);

				addComponent<ecs::TransformComponent>(
						e, 
						transform
					);
#ifdef CLZ_ENABLE_EDITOR
				/// @brief If Editor is enabled,
				/// only then create these components
				auto editorTransform = ecs::EditorTransformComponent(transform);
				ecs::addComponent<ecs::EditorTransformComponent>(
					e,
					editorTransform
				);
#endif
			}

			// Attach ModelComponent if present
			if (entityData.contains("model"))
			{
				ecs::addComponent<renderer::ModelComponent>(
					e,
					retrieveModelComponent(entityData["model"]["path"], entityName)
				);
			}
			// Attach Dir light Component if present
			if (entityData.contains("directionallight"))
			{
				const auto result = retrieveDirectionalLightComponent(
					entityData["directionallight"],
					entityName
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
					retrievePointLightComponent(entityData["pointlight"], entityName);
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
				auto body = retrieveBodyComponent(entityData["rigidbody"], e, entityName);
				ecs::addComponent<physics::RigidBodyComponent>(e, body);
			}


			// Attach sensor script component, if present
			if (entityData.contains("sensor_scripts"))
			{
				ecs::addComponent<script::SensorScriptComponent>(
					e, 
					retrieveSensorScriptComponent(
						entityData["sensor_scripts"])
				);
			}
			// Attach collision script component, if present
			if (entityData.contains("collision_scripts"))
			{
				ecs::addComponent<script::CollisionScriptComponent>(
					e, 
					retrieveCollisionScriptComponent(
						entityData["collision_scripts"])
				);
			}
			
			// Attach buffer player component, if present
			if (entityData.contains("buffer_player"))
			{
				ecs::addComponent<audio::AudioBufferPlayerComponent>(
					e,
					retrieveBufferPlayerComponent(
						entityData["buffer_player"],
						e
					)
				);
			}
		}

		// Entities loaded flag
		renderer::flagRenderComponentsLoaded();
		script::flagScriptSystemEntitiesLoaded();

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
			std::string entityName = ecs::getEntityName(entity);
			entityJson["name"] = entityName;

			/// --- 1. Transform components ---
			if (ecs::hasComponent<ecs::TransformComponent>(entity))
			{
				saveTransformComponent(
					ecs::getComponent<ecs::TransformComponent>(entity),
					entityJson["transform"],
					entityName
				);
			}

			/// --- 2. Renderer components ---
			if (ecs::hasComponent<renderer::ModelComponent>(entity))
			{
				saveModelComponent(
					ecs::getComponent<renderer::ModelComponent>(entity),
					entityJson["model"],
					entityName
				);
			}
			if (ecs::hasComponent<renderer::DirectionalLightComponent>(entity))
			{
				saveDirectionalLightComponent(
					ecs::getComponent<renderer::DirectionalLightComponent>(
						entity
					),
					entityJson["directionallight"],
					entityName
				);
			}
			if (ecs::hasComponent<renderer::PointLightComponent>(entity))
			{
				savePointLightComponent(
					ecs::getComponent<renderer::PointLightComponent>(entity),
					entityJson["pointlight"],
					entityName
				);
			}

			/// --- 3. Physics components --- ///
			if (ecs::hasComponent<physics::RigidBodyComponent>(entity))
			{
				saveRigidBodyComponent(
					ecs::getComponent<physics::RigidBodyComponent>(entity),
					entityJson["rigidbody"],
					entityName
				);
			}

			/// --- 4. Sensor script components --- ///
			if (ecs::hasComponent<script::SensorScriptComponent>(entity))
			{
				saveSensorScriptComponent(
					ecs::getComponent<script::SensorScriptComponent>(entity),
					entityJson["sensor_scripts"]
				);
			}
			/// --- 4. Sensor script components --- ///
			if (ecs::hasComponent<script::CollisionScriptComponent>(entity))
			{
				saveCollisionScriptComponent(
					ecs::getComponent<script::CollisionScriptComponent>(entity),
					entityJson["collision_scripts"]
				);
			}


			/// --- 6. Buffer player components --- ///
			if (ecs::hasComponent<audio::AudioBufferPlayerComponent>(entity))
			{
				saveBufferPlayerComponent(
					ecs::getComponent<audio::AudioBufferPlayerComponent>(entity),
					entityJson["buffer_player"]
				);
			}
			sceneJson["entities"].push_back(entityJson);
		}
	}

} // namespace clz::scene
