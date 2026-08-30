/**
 * @file loader.hpp
 * @author curl0z
 * @brief Header file containing definition of loading entity components from JSON
 * or writing back to it.
 * All Components have their seperate implementation file in src/scene/componentloader
 * directory
 */
#pragma once

#include "entity/corecomponents.hpp"
#include "entity/entitymanager.hpp"
#include "physics/physicscomponent.hpp"
#include "renderer/rendercomponent.hpp"
#include <nlohmann/json.hpp>

namespace clz::scene
{
	/// @brief Retrieves transform component of any entity.
	/// @param componentData JSON index of entity
	/// @param entityName Entity's name
	/// @return TransformComponent of given entity
	ecs::TransformComponent retrieveTransformComponent(
			const nlohmann::json& componentData, 
			std::string_view entityName);

	/**
	 * @brief Saves transform component of any entity.
	 * @param tc TransformComponent of given entity
	 * @param componentData JSON index of entity
	 * @param entityName Entity's name
	 */
	void saveTransformComponent(
			const ecs::TransformComponent& tc, 
			nlohmann::json& componentData,
			std::string_view entityName);

	/**
	 * @brief Retrieves Model component of any entity.
	 * @param path of entity's model
	 * @param entityName Entity's name
	 * @return ModelComponent of given entity
	 */
	renderer::ModelComponent retrieveModelComponent(
			const std::filesystem::path& path, 
			std::string_view entityName);
	/**
	 * @brief Saves model component of any entity.
	 * @param mc modelComponent of given entity
	 * @param componentData JSON index of entity
	 * @param entityName Entity's name
	 */
	void saveModelComponent(
			const renderer::ModelComponent& mc, 
			nlohmann::json& componentData,
			std::string_view entityName);

	/// @brief Retrieves directional light component
	/// @param componentData Component field in JSON(simply pass the
	/// entity["directionallight"]
	/// @param entityName Entity's name
	/// @return DirectionalLightComponent If registering was successful, else will return
	/// error message.
	/// @note Always print the error, and ignore this component, don't assert
	std::expected<renderer::DirectionalLightComponent, std::string>
	retrieveDirectionalLightComponent(
			const nlohmann::json& componentData,
			std::string_view entityName);
	/// @brief writes back dir light componenet data back to JSON
	/// @param dlt dir light component
	/// @param componentData which field to save back to
	/// @param entityName Entity's name
	void saveDirectionalLightComponent(
		const renderer::DirectionalLightComponent& dlt,
		nlohmann::json& componentData,
		std::string_view entityName);

	/// @brief Retrieves point light component
	/// @param componentData Component field in JSON(simply pass the entity["pointlight"]
	/// @param entityName Entity's name
	/// @return PointLightComponent If registering was successful, else will return error
	/// message.
	/// @note Always print the error, and ignore this component, don't assert
	std::expected<renderer::PointLightComponent, std::string>
	retrievePointLightComponent(
			const nlohmann::json& componentData,
			std::string_view entityName);
	/// @brief writes back point light componenet data back to JSON
	/// @param plc point light component
	/// @param componentData which field to save back to
	/// @param entityName Entity's name
	void savePointLightComponent(
		const renderer::PointLightComponent& plc,
		nlohmann::json& componentData,
		std::string_view entityName);

	/// @brief Loads all body and shapes from JSON
	/// @param physicsTable Physics table in JSON file
	/// @param entity Entity for which we are creating this entity
	/// @param entityName Entity's name
	/// @return RigidBodyComponent of entity
	/// @note if a value is not present in JSON, will assign default value
	physics::RigidBodyComponent
	retrieveBodyComponent(
			const nlohmann::json& physicsTable,
			const ecs::entity& entity,
			std::string_view entityName);

	/// @brief Saves back all physics data of entities to JSON
	/// @param rigidBodyComponent RigidBodyComponent of entity
	/// @param physicsTable JSON-array where we have to write back data
	/// @param entityName Entity's name
	void saveRigidBodyComponent(
		physics::RigidBodyComponent rigidBodyComponent,
		nlohmann::json& physicsTable,
		std::string_view entityName
	);

} // namespace clz::scene
