// Loader is one header file, distributed across multiple files
#pragma once

#include "../components.hpp"
#include <nlohmann/json.hpp>
#include <tuple>

namespace clz::ecs
{
	/**
	 * @brief Retrieves transform component of any entity.
	 * @param componentData JSON index of entity
	 * @return TransformComponent of given entity
	 */
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

	std::tuple<RigidBodyComponent, RigidBodyDataComponent> retrieveBodyComponent(const nlohmann::json& physicsTable,
											const TransformComponent& tc);

	void saveRigidBodyComponent(const std::tuple<RigidBodyComponent, RigidBodyDataComponent>& rigidBodyComponent,
											nlohmann::json& componentData);

}

