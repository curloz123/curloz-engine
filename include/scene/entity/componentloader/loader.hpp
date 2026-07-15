/**
 * @file loader.hpp
 * @author curl0z
 * @brief Header file containing definition of loading entity components from JSON
 * or writing back to it.
 * All Components have their seperate implementation file in src/scene/componentloader directory
 */
#pragma once

#include "../components.hpp"
#include <nlohmann/json.hpp>
#include <tuple>

namespace clz::ecs
{
	/// @brief Retrieves transform component of any entity.
	/// @param componentData JSON index of entity
	/// @return TransformComponent of given entity
	TransformComponent retrieveTransformComponent(const nlohmann::json& componentData);

	/**
	 * @brief Saves transform component of any entity.
	 * @param tc TransformComponent of given entity
	 * @param componentData JSON index of entity
	 */
	void saveTransformComponent(const TransformComponent& tc, nlohmann::json& componentData);


	/**
	 * @brief Retrieves Model component of any entity.
	 * @param path of entity's model
	 * @return ModelComponent of given entity
	 */
	ModelComponent retrieveModelComponent(const std::filesystem::path& path);

	/**
	 * @brief Saves model component of any entity.
	 * @param mc modelComponent of given entity
	 * @param componentData JSON index of entity
	 */
	void saveModelComponent(const ModelComponent& mc, nlohmann::json& componentData);


	/// @brief Loads all body and shapes from JSON
	/// @param physicsTable Physics table in JSON file
	/// @param entity Entity for which we are creating this entity
	/// @return std::tuple<BodyComponent, ShapeComponent> Both components
	/// @note if a value is not present in JSON, will assign default value
	std::tuple<BodyComponent, ShapeComponent>
	retrieveBodyComponent(const nlohmann::json& physicsTable, const entity& entity);

	/// @brief Saves back all physics data of entities to JSON
	/// @param rigidBodyComponent Tuple containing both BodyComponent and ShapeComponent of entity
	/// @param physicsTable JSON-array where we have to write back data
	void saveRigidBodyComponent(const std::tuple<BodyComponent,
		ShapeComponent>& rigidBodyComponent, nlohmann::json& physicsTable);

}

