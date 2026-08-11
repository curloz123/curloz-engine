/**
 * @file entitymanager.cpp
 * @author curl0z
 * @brief Implementation file of entity manager
 * Contains function for creating, destroying etc.
 * Any function related with entities.
 */

#include "entity/entitymanager.hpp"
#include "entity/componentmanager.hpp"
#include "entity/tags.hpp"
#include "physics/physicscomponent.hpp"
#include "renderer/rendercomponent.hpp"

namespace clz::ecs
{

	/// @copydoc createEntity
	uint32_t createEntity(const std::string& name)
	{
		const uint32_t index = entityCounter++;
		entities.emplace_back(index);
		entityName.emplace_back(name);
		return index;
	}

	/// @copydoc removeEntity
	void removeEntity(const entity e)
	{
		removeAllComponentsForEntity(e);
		entities[e] = NULL_ENTITY;
		entityName[e] = "INVALID_ENTITY";
	}

	/// @copydoc clearEntities
	void clearEntities()
	{
		for (const auto entity : entities)
		{
			removeAllComponentsForEntity(entity);
		}
		entities.clear();
	}

	/// @copydoc getEntities
	const std::vector<entity>& getEntities()
	{
		return entities;
	}

	/// @copydoc getEntityName
	std::string getEntityName(const entity e)
	{
		if (entities[e] == NULL_ENTITY)
		{
			clz::log::warn(
				"attempt to retrieve "
				"name of an non existing"
				"id: " +
				std::to_string(e)
			);
			return NULL_ENTITY_NAME;
		}
		return entityName[e];
	}

	/// @copydoc disableEntity
	void disableEntity(const entity e)
	{
		addComponent<DisableTagComponent>(e, DisableTagComponent());
		if (hasComponent<renderer::ModelComponent>(e))
		{
			disableComponent<renderer::ModelComponent>(e);
		}
		if (hasComponent<physics::RigidBodyComponent>(e))
		{
			disableComponent<physics::RigidBodyComponent>(e);
		}
	}

	/// @copydoc enableEntity
	void enableEntity(const entity e)
	{
		removeComponent<DisableTagComponent>(e);
		if (hasComponent<renderer::ModelComponent>(e))
		{
			enableComponent<renderer::ModelComponent>(e);
		}
		if (hasComponent<physics::RigidBodyComponent>(e))
		{
			enableComponent<physics::RigidBodyComponent>(e);
		}
	}

	/// @copydoc isEntityDisabled
	bool isEntityDisabled(const entity e)
	{
		return hasComponent<DisableTagComponent>(e);
	}

	/// @copydoc markEntityForDeletion
	void markEntityForDeletion(const entity e)
	{
		addComponent<DeletionTagComponent>(e, DeletionTagComponent());
		disableEntity(e);
	}

	/// @copydoc unMarkEntityForDeletion
	void unMarkEntityForDeletion(const entity e)
	{
		removeComponent<DeletionTagComponent>(e);
		enableEntity(e);
	}

	/// @copydoc isMarkedForDeletion
	bool isMarkedForDeletion(const entity e)
	{
		return hasComponent<DeletionTagComponent>(e);
	}

} // namespace clz::ecs