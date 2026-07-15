/**
 * @file entity.hpp
 * @author curl0z
 * @brief Main file for entity system,
 * Handles all loading, saving, destroying process for entities
 */

#pragma once

#include <nlohmann/json.hpp>

namespace clz::ecs
{
	/**
	 * @brief Loads all entities from JSON file
	 * @param entityJson JSON's entity-table containing entity data
	 */
	bool loadEntities(const nlohmann::json& entityJson);

	/**
	 * @brief Writes back entity scene data to JSON file
	 * @param entityJson JSON Entity table
	 */
	void saveEntities(nlohmann::json& entityJson);

	/**
	 * @brief Destroy's all entities
	 * Clear's out all it's components too!!
	 */
	void destroyEntities();
} // namespace clz::ecs