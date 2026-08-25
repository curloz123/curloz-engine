/**
 * @file componentstorage.hpp
 * @author curl0z
 * @brief Sparse set based component storage.
 *
 * Each component type gets its own ComponentStorage<T> instance.
 * Uses a sparse set for O(1) add, remove, and lookup, with
 * cache friendly contiguous iteration via the dense array.
 *
 * Layout:
 *   storage[i] — component data, contiguous
 *   dense[i]   — entity that owns storage[i]
 *   sparse[e]  — index into storage/dense for entity e, NULL_ENTITY if absent
 */

#pragma once

#include "entitymanager.hpp"
#include <vector>

namespace clz::ecs
{
	/**
	 * @brief Non-templated base for ComponentStorage.
	 *
	 * Exists solely so different ComponentStorage<T> types can be stored
	 * in one container. Only one virtual method — removeEntityData —
	 * called only on entity destruction, not in hot loops.
	 */
	struct IComponentStorage
	{
		virtual ~IComponentStorage() = default;

		/**
		 * @brief Removes the component for entity @p e if it exists.
		 * @param e Entity to remove component data for.
		 */
		virtual void removeEntityData(entity e) = 0;
	};

	/// @brief In sparse array, if index is not defined, simply use this
	inline constexpr uint32_t NULL_STORAGE_INDEX = UINT32_MAX;

	/**
	 * @brief Typed sparse set storage for component type T.
	 *
	 * @tparam T Component type to store.
	 *
	 * @note Do not access storage, dense, or sparse directly from systems.
	 * Use componentmanager.hpp functions instead.
	 */
	template <typename T> struct ComponentStorage : IComponentStorage
	{
		std::vector<T> storage; ///< Contiguous component data. Iterate this for hot paths.
		std::vector<entity> dense;    ///< Entity that owns storage[i]. Parallel to storage.
		std::vector<uint32_t> sparse; ///< index into storage. NULL_ENTITY if absent.

		/// @brief Initializes sparse array to NULL_ENTITY for MAX_ENTITIES slots.
		ComponentStorage()
		{
			sparse.resize(MAX_ENTITIES, NULL_STORAGE_INDEX);
		}

		/**
		 * @brief Removes component data for entity @p e using swap-and-pop.
		 *
		 * Swaps the target with the last element then pops — O(1), no shifting.
		 * Updates sparse for the moved entity to keep indices valid.
		 *
		 * @param e Entity whose component data to remove.
		 */
		void removeEntityData(const entity e) override
		{
			if (sparse[e] == NULL_STORAGE_INDEX)
				return;

			uint32_t index = sparse[e];
			uint32_t last = storage.size() - 1;

			// swap target with last
			storage[index] = storage[last];
			dense[index] = dense[last];
			sparse[dense[index]] = index; // update moved entity's sparse entry

			dense.pop_back();
			storage.pop_back();
			sparse[e] = NULL_STORAGE_INDEX;
		}
	};

} // namespace clz::ecs