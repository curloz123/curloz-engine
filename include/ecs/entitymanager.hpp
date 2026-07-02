/**
 * @file entitymanager.hpp
 * @author curl0z
 * @brief Entity creation and destruction functions.
 *
 * Entities are just integer IDs. All data lives in component
 * storages in componentmanager.hpp.
 */

#pragma once

#include "componentmanager.hpp"
#include "entity.hpp"
#include <cstdint>
#include <vector>

namespace clz::ecs
{
	/**
	 * @brief Creates a new entity and returns its ID.
	 *
	 * Entity ID is the current size of ecs_entities
	 *
	 * @return Unique entity ID.
	 */
	inline uint32_t createEntity(const std::string& name)
	{
		uint32_t id = entities.size();
		entities.emplace_back(id);
		entityName.emplace_back(name);
		return id;
	}

	/**
	 * @brief Removes an entity and all its components.
	 *
	 * Strips all components via removeAllComponentsForEntity.
	 * The entity ID slot in ecs_entities is not recycled tho.
	 * Maybe added later.
	 *
	 * @param e Entity to remove.
	 */
	inline void removeEntity(const entity e)
	{
		removeAllComponentsForEntity(e);
		entities[e] = NULL_ENTITY;
		entityName[e] = "INVALID_ENTITY";
	}

	/**
	 * @brief Clears the entity list without destroying components.
	 *
	 * @note Call deleteAllComponents() before or after this to
	 * avoid dangling component data.
	 */
	inline void clearEntities()
	{
		entities.clear();
	}

	/**
	 * @brief Returns a const reference to entity array
	 * @return entity(uint32_t) vector
	 */
	inline const std::vector<uint32_t>& getEntities()
	{
		return entities;
	}

	/**
	 * @brief Returns the name of entity
	 * @return name string
	 */
	inline std::string getEntityName(const entity e)
	{
		return entityName[e];
	}



} // namespace clz::ecs