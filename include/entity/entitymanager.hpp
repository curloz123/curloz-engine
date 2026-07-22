/**
 * @file entitymanager.hpp
 * @author curl0z
 * @brief Handles creation and destruction of entities
 *
 * An entity is just a unique integer ID. All data lives in
 * component storages indexed by this ID.
 *
 */

#pragma once

#include "scene/entity/entity.hpp"
#include <cstdint>
#include <array>

namespace clz::ecs
{
	/// @brief Entity handle
	/// just a unique integer ID.
	using entity = uint32_t;

	/// @brief Sentinel value representing an invalid or null entity.
	inline constexpr entity NULL_ENTITY = std::numeric_limits<uint32_t>::max();
	/// @brief Initial sparse array size. Grows dynamically as needed.
	inline constexpr entity MAX_ENTITIES = 256;
	/// @brief All entity IDs. Index is not the same as entity ID after removals.
	inline std::vector<entity> entities;
	/// @brief valid entity count
	inline size_t entityCounter;

	/// @brief Null name
	inline constexpr auto NULL_ENTITY_NAME = "NULL_ENTITY_NAME";
	/// @brief All entity's names
	inline std::vector<std::string> entityName;

	/**
	 * @brief Creates a new entity and returns its ID.
	 *
	 * Entity ID is the current size of ecs_entities
	 *
	 * @return Unique entity ID.
	 */
	uint32_t createEntity(const std::string& name);

	/**
	 * @brief Removes an entity and all its components.
	 *
	 * Strips all components via removeAllComponentsForEntity.
	 * The entity ID slot in ecs_entities is not recycled tho.
	 * Maybe added later.
	 *
	 * @param e Entity to remove.
	 */
	void removeEntity(entity e);

	/**
	 * @brief Clears the entity list also automatically
	 * destroying the components associated with them.
	 */
	void clearEntities();

	/**
	 * @brief Returns a const reference to entity array
	 * @return entity(uint32_t) vector
	 */
	const std::vector<entity>& getEntities();

	/**
	 * @brief Returns the name of entity
	 * @return name string
	 */
	std::string getEntityName(entity e);

} // namespace clz::ecs