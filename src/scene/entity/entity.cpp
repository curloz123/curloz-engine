/**
 * @file entity.cpp
 * @author curl0z
 * @brief Entity subsystem implementation
 */

#include "scene/entity/entity.hpp"
#include "../../../include/renderer/entitydata/modeldata.hpp"
#include "include/offscreen/offscreentarget.hpp"
#include "math/quateulerconv.hpp"
#include "renderer/rendercomponent.hpp"
#include "scene/entity/componentloader/loader.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"
#include "scene/entity/entitymanager.hpp"

namespace clz::ecs
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
	 * @note It is necessary for each entity to have a name, if not it will be marked "Unnamed Entity"
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
				e = createEntity("Unnamed Entity");
			}
			else
			{
				e = createEntity(entityData["name"]);
			}

			// Attach TransformComponent, should be present on every entity
			if (!entityData.contains("transform"))
			{
				clz::log::warn("Entity: " + entityName[e] +
					       "Does not have transform component"
					       "Assigning it identity transform component");
				addComponent<TransformComponent>(e, TransformComponent());
			}
			else
			{
				const auto transform = retrieveTransformComponent(entityData["transform"]);
				addComponent<TransformComponent>(e, transform);
#ifdef CLZ_ENABLE_EDITOR
				/// @brief If Editor is enabled,
				/// only then create these components
				addComponent<EulerRotationComponent>(e, EulerRotationComponent(math::quatToEulerXYZ(transform.rotation)));
				addComponent<EditorTransformComponent>(e, EditorTransformComponent(transform));
#endif
			}

			// Attach ModelComponent if present
			if (entityData.contains("model"))
			{
				addComponent<ModelComponent>(e, retrieveModelComponent(entityData["model"]["path"]));
			}

			// Attach physics Component if present
			if (entityData.contains("rigidbody"))
			{
				const auto& [body, shapes] = retrieveBodyComponent(entityData["rigidbody"], e);
				addComponent<BodyComponent>(e, body);
				addComponent<ShapeComponent>(e, shapes);
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

			if (hasComponent<BodyComponent>(entity))
			{
				saveRigidBodyComponent(std::make_tuple(getComponent<BodyComponent>(entity), getComponent<ShapeComponent>(entity)),
						       entityJson["rigidbody"]);
			}
			sceneJson["entities"].push_back(entityJson);
		}
	}

	/// @brief destroys all entites after they have been saved
	void destroyEntities()
	{
		clearEntities();
		clz::log::info("Destroyed Entities");
	}
} // namespace clz::ecs