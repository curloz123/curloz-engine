/**
 * @file componentmanager.hpp
 * @author curl0z
 * @brief Component storage registry and component operation functions.
 *
 * Manages all ComponentStorage instances via a flat vector indexed
 * by component ID. Component IDs are assigned at runtime on first
 * call to componentID<T>().
 *
 * Hot path functions: getStorage, addComponent, removeComponent, getComponent.
 * Cold path functions: deleteAllComponents, removeAllComponentsForEntity.
 */

#pragma once

#include "componentstorage.hpp"
#include "core/logs.hpp"
#include "entitymanager.hpp"
#include <algorithm>

namespace clz::ecs
{
	/// @brief Flat array of all component storages, indexed by component ID.
	inline std::vector<IComponentStorage*> componentStorages;

	/// @brief Auto-incrementing counter for assigning unique component IDs.
	inline uint32_t componentCounter = 0;

} // namespace clz::ecs

namespace clz::ecs
{
	/**
	 * @brief Returns the unique integer ID for component type T.
	 *
	 * ID is assigned on first call and remains constant for the
	 * lifetime of the program.
	 *
	 * @tparam T Component type.
	 * @return Unique integer ID for T.
	 */
	template <typename T> uint32_t componentID()
	{
		static uint32_t id = componentCounter++;
		return id;
	}

	/**
	 * @brief Returns the ComponentStorage for component type T.
	 *
	 * Creates the storage on first call. Uses a flat vector indexed
	 * by component ID.
	 *
	 * @tparam T Component type.
	 * @return Reference to the ComponentStorage<T> for T.
	 */
	template <typename T> ComponentStorage<T>& getStorage()
	{
		auto id = componentID<T>();
		if (id >= componentStorages.size())
		{
			componentStorages.resize(id + 1, nullptr);
			componentStorages[id] = new ComponentStorage<T>;
		}
		return static_cast<ComponentStorage<T>&>(*componentStorages[id]);
	}

	/**
	 * @brief Destroys all component storages and clears the registry.
	 *
	 * @note Call this in shutdown before clearing entities.
	 */
	inline void deleteAllComponents()
	{
		for (auto& storage : componentStorages)
			delete storage;
		componentStorages.clear();
	}

	/**
	 * @brief Attaches a component to an entity.
	 *
	 * @tparam T Component type.
	 * @param e Entity to attach the component to.
	 * @param component Component data to store.
	 */
	template <typename T> void addComponent(entity e, T component)
	{
		auto& componentStorage = getStorage<T>();
		uint32_t index = componentStorage.storage.size();
		componentStorage.storage.emplace_back(component);
		componentStorage.dense.emplace_back(e);
		componentStorage.sparse[e] = index;
	}

	/**
	 * @brief Returns true if entity @p e has component T.
	 *
	 * @tparam T Component type.
	 * @param e Entity to check.
	 * @return True if the entity has the component, false otherwise.
	 */
	template <typename T> bool hasComponent(entity e)
	{
		auto& componentStorage = getStorage<T>();
		return componentStorage.sparse[e] != NULL_STORAGE_INDEX;
	}

	/**
	 * @brief Removes component T from entity @p e using swap-and-pop.
	 *
	 * No-op if the entity does not have the component.
	 *
	 * @tparam T Component type.
	 * @param e Entity to remove the component from.
	 */
	template <typename T> void removeComponent(entity e)
	{
		auto& componentStorage = getStorage<T>();
		if (componentStorage.sparse[e] == NULL_STORAGE_INDEX)
		{
			clz::log::warn("Attempt to remove a non existing component");
			return;
		}
		componentStorage.removeEntityData(e);
	}

	/**
	 * @brief Set's a component of an entity to @p value
	 * Mainly useful for editor undo-redo
	 * @tparam T Component type
	 * @param e Entity to set component
	 * @param component the actual component
	 */
	template <typename T> void setComponent(entity e, const T& component)
	{
		auto& componentStorage = getStorage<T>();
		componentStorage.storage[componentStorage.sparse[e]] = component;
	}

	/**
	 * @brief Removes all components from entity @p e across all storages.
	 *
	 * Iterates all registered storages and calls removeEntityData.
	 * Called by removeEntity and shutdown.
	 *
	 * @param e Entity to strip all components from.
	 */
	inline void removeAllComponentsForEntity(const entity e)
	{
		for (const auto& storage : componentStorages)
			storage->removeEntityData(e);
	}

	/**
	 * @brief Returns a reference to component T for entity @p e.
	 *
	 * @tparam T Component type.
	 * @param e Entity to retrieve the component for.
	 * @return Reference to the component in storage.
	 * @warning Undefined behaviour if the entity does not have component T.
	 */
	template <typename T> T& getComponent(entity e)
	{
		auto& componentStorage = getStorage<T>();
		return componentStorage.storage[componentStorage.sparse[e]];
	}

	/**
	 * @brief Returns all entities that have the asked component.
	 *
	 * Returns a const reference to the internal dense array.
	 *
	 * @tparam T Component type to query.
	 * @return Const reference to the dense entity array for asked component.
	 *
	 * @warning The returned reference is invalidated if any component
	 * of type T is added or removed during iteration. Never add or
	 * remove components of type T while iterating the result.
	 */
	template <typename T> const std::vector<entity>& getEntitiesWithComponent()
	{
		auto& componentStorage = getStorage<T>();

		return componentStorage.dense;
	}

	template <typename T> std::vector<T>& getComponentArray()
	{
		auto& componentStorage = getStorage<T>();
		return componentStorage.storage;
	}

	/**
	 * @brief Returns references to multiple components for entity @p e.
	 *
	 * Returns a tuple of references. Modifications affect
	 * the actual storage data.
	 *
	 * @note Don't use this function directly in hot loops.
	 * Tuple construction adds overhead not suitable for tight iteration.
	 *
	 * @tparam Components Component types to retrieve.
	 * @param e Entity to retrieve components for.
	 * @return std::tuple of references to each requested component.
	 */
	template <typename... Components> std::tuple<Components&...> getComponents(entity e)
	{
		return std::tie(getComponent<Components>(e)...);
	}

	template <typename T> void disableComponent(const entity e)
	{
		auto& componentStorage = getStorage<T>();
		if (componentStorage.sparse[e] == NULL_STORAGE_INDEX)
		{
			log::warn(
				"Attempt to disable non existing component"
				"for entity: " +
				std::to_string(static_cast<uint32_t>(e))
			);
			return;
		}

		auto it =
			std::find(componentStorage.dense.begin(), componentStorage.dense.end(), e);
		if (it != componentStorage.dense.end())
		{
			componentStorage.dense.erase(it);
		}
	}

	template <typename T> void enableComponent(const entity e)
	{
		auto& componentStorage = getStorage<T>();
		if (componentStorage.sparse[e] == NULL_STORAGE_INDEX)
		{
			log::warn(
				"Attempt to enable non existing component"
				"for entity: " +
				getEntityName(e)
			);
			return;
		}

		componentStorage.dense.emplace_back(e);
		std::sort(
			componentStorage.dense.begin(),
			componentStorage.dense.end(),
			[](const entity e1, const entity e2) {
				return static_cast<uint32_t>(e1) < static_cast<uint32_t>(e2);
			}
		);
	}

} // namespace clz::ecs