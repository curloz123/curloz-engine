/**
 * @file entity.hpp
 * @author curl0z
 * @brief Main file for entity system
 */

#pragma once

#include <nlohmann/json.hpp>

namespace clz::ecs
{
	/**
	 * @brief Loads all entities from JSON file
	 * @param entityJson nlohmann::json object that has already parsed the JSON
	 */
	bool loadEntities(const nlohmann::json& entityJson);

	/**
	 * @brief Writes back entity scene data to JSON file
	 * @param entityJson Json Entity table
	 */
	void saveEntities(nlohmann::json& entityJson);

	/**
	 * @brief Destroy's all entities
	 * Clear's out all it's components too!!
	 */
	void destroyEntities();
} // namespace clz::ecs