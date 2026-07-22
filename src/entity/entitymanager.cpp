/**
 * @file entitymanager.cpp
 * @author curl0z
 * @brief Implementation file of entity manager
 * Contains function for creating, destroying etc.
 * Any function related with entities.
 */

#include "entity/entitymanager.hpp"
#include "entity/componentmanager.hpp"

namespace clz::ecs
{
	uint32_t createEntity(const std::string& name)
	{
		const uint32_t index = entityCounter++;
		entities.emplace_back(index);
		entityName.emplace_back(name);
		return index;
	}
	void removeEntity(const entity e)
	{
		removeAllComponentsForEntity(e);
		entities[e] = NULL_ENTITY;
		entityName[e] = "INVALID_ENTITY";
	}

	void clearEntities()
	{
		for (const auto entity : entities)
		{
			removeAllComponentsForEntity(entity);
		}
		entities.clear();
	}

	const std::vector<entity>& getEntities()
	{
		return entities;
	}

	std::string getEntityName(const entity e)
	{
		if (entities[e] == NULL_ENTITY)
		{
			clz::log::warn("attempt to retrieve "
					"name of an non existing"
					"id: " + std::to_string(e));
			return NULL_ENTITY_NAME;
		}
		return entityName[e];
	}

} // namespace clz::ecs