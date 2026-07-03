/**
 * @file entitymanager.cpp
 * @author curl0z
 * @brief Implementation file of entity manager
 * Contains function for creating, destroying etc.
 * Any function related with entities.
 */

#include "scene/entity/entitymanager.hpp"
#include "scene/entity/componentmanager.hpp"

namespace clz::ecs
{
	uint32_t createEntity(const std::string& name)
	{
		uint32_t id = entities.size();
		entities.emplace_back(id);
		entityName.emplace_back(name);
		return id;
	}
	void removeEntity(const entity e)
	{
		removeAllComponentsForEntity(e);
		entities[e] = NULL_ENTITY;
		entityName[e] = "INVALID_ENTITY";
	}

	void clearEntities()
	{
		for (auto entity : entities)
		{
			removeAllComponentsForEntity(entity);
		}
		entities.clear();
	}

	const std::vector<uint32_t>& getEntities()
	{
		return entities;
	}

	std::string getEntityName(const entity e)
	{
		return entityName[e];
	}

} // namespace clz::ecs