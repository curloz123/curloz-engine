/**
 * @file transformloader.cpp
 * @author curl0z
 * @brief Loads/Save transform data of an entity from/to JSON
 * @note not required to pass entity, just pass their data!!
 */
#include "core/logs.hpp"
#include "scene/entity/loader.hpp"

namespace clz::scene
{
	/// @copydoc retrieveTransformComponent
	ecs::TransformComponent retrieveTransformComponent(
			const nlohmann::json& componentData,
			std::string_view entityName)
	{
		clz::math::quat q(1.0f, 0.0f, 0.0f, 0.0f);
		if (componentData.contains("rotation"))
		{
			q = clz::math::quat(
				componentData["rotation"][0],
				componentData["rotation"][1],
				componentData["rotation"][2],
				componentData["rotation"][3]
			);
		}
		else
		{
			clz::log::warn("Entity: " + std::string(entityName) + 
					" has transform component but no rotation component" + 
					" Assigning some random value");
		}

		clz::math::vec3 t(0.0f, 0.0f, 0.0f);
		if (componentData.contains("position"))
		{
			t = clz::math::vec3(
				componentData["position"][0],
				componentData["position"][1],
				componentData["position"][2]
			);
		}
		else
		{
			clz::log::warn("Entity: " + std::string(entityName) + 
					" has transform component but no position part" + 
					" Assigning some random value");
		}

		clz::math::vec3 s(1.0f, 1.0f, 1.0f);
		if (componentData.contains("scale"))
		{
			s = clz::math::vec3(
				componentData["scale"][0],
				componentData["scale"][1],
				componentData["scale"][2]
			);
		}
		else
		{
			clz::log::warn("Entity: " + std::string(entityName) + 
					" has transform component but no scale component" + 
					" Assigning some random value");
		}

		return {q, t, s};
	}

	/// @copydoc saveTransformComponent
	void saveTransformComponent(
			const ecs::TransformComponent& tc,
			nlohmann::json& componentData,
			std::string_view entityName)
	{
		componentData["rotation"][0] = tc.rotation.w;
		componentData["rotation"][1] = tc.rotation.x;
		componentData["rotation"][2] = tc.rotation.y;
		componentData["rotation"][3] = tc.rotation.z;
		componentData["position"][0] = tc.position.x;
		componentData["position"][1] = tc.position.y;
		componentData["position"][2] = tc.position.z;
		componentData["scale"][0] = tc.scale.x;
		componentData["scale"][1] = tc.scale.y;
		componentData["scale"][2] = tc.scale.z;

		clz::log::info("Saved transform component for entity " + std::string(entityName));
	}
} // namespace clz::scene
