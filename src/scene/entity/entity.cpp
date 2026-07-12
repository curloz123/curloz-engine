/**
 * @file entity.cpp
 * @author curl0z
 * @brief Entity subsystem implementation
 */

#include "scene/entity/entity.hpp"
#include "../../../include/renderer/entitydata/modeldata.hpp"
#include "include/editor.hpp"
#include "include/offscreen/offscreentarget.hpp"
#include "math/quateulerconv.hpp"
#include "renderer/rendercomponent.hpp"
#include "scene/entity/componentloader/loader.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"
#include "scene/entity/entitymanager.hpp"

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
				addComponent<TransformComponent>(e, retrieveTransformComponent(entityData["transform"]));
			}
#ifdef CLZ_ENABLE_EDITOR
			addComponent<EulerRotationComponent>(e, {math::quatToEulerXYZ(getComponent<TransformComponent>(e).rotation)});
#endif

			// Attach ModelComponent if present
			if (entityData.contains("model"))
			{
				addComponent<ModelComponent>(e, retrieveModelComponent(entityData["model"]["path"]));
			}

			// Attach physics Component if present
			if (entityData.contains("rigidbody"))
			{
				auto [body, data] =
					retrieveBodyComponent(entityData["rigidbody"], ecs::getComponent<TransformComponent>(e));
				addComponent<RigidBodyComponent>(e, body);
				addComponent<RigidBodyDataComponent>(e, data);
			}
		}

		// Entities loaded flag
		renderer::flagRenderComponentsLoaded();
		editor::flagOffscreenTargetsEntitiesLoaded();

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

			if (hasComponent<RigidBodyDataComponent>(entity))
			{
				saveRigidBodyComponent(std::make_tuple(getComponent<RigidBodyComponent>(entity),
					getComponent<RigidBodyDataComponent>(entity)), entityJson["rigidbody"]);
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