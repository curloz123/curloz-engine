/**
 * @file entity.hpp
 * @author curl0z
 * @brief Handles creation and destruction of entities
 *
 * An entity is just a unique integer ID. All data lives in
 * component storages indexed by this ID.
 */

#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include <string>

namespace clz::ecs
{
	/// @brief Entity handle
	/// just a unique integer ID.
	using entity = uint32_t;

	/// @brief Sentinel value representing an invalid or null entity.
	constexpr entity NULL_ENTITY = std::numeric_limits<uint32_t>::max();

	/// @brief Initial sparse array size. Grows dynamically as needed.
	constexpr entity MAX_ENTITIES = 256;

	/// @brief All entity IDs. Index is not the same as entity ID after removals.
	inline std::vector<entity> entities;

	/// @brief Components of all entities
	inline std::vector<uint32_t> entityComponents;

	/// @brief All entity's names
	inline std::vector<std::string> entityName;

} // namespace clz::ecs