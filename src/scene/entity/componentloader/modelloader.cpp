/**
 * @file modelloader.cpp
 * @author curl0z
 * @brief Loads model component of an entity
 */

#include "core/logs.hpp"
#include "entity/entitymanager.hpp"
#include "renderer/model/model.hpp"
#include "renderer/rendercomponent.hpp"
#include "scene/entity/loader.hpp"

namespace clz::scene
{
	/// @copydoc retrieveModelComponent
	renderer::ModelComponent retrieveModelComponent(
		const nlohmann::json& modelCompEntry, 
		const ecs::entity entt
	)
	{
		const auto name = ecs::getEntityName(entt);
		if (!modelCompEntry.contains("index") || 
			modelCompEntry["index"].is_null()
		)
		{
			clz::log::warn(
				name +
				"'s model entry (does not have)/(has null)"
				" index field, attaching null component"
			);
			return renderer::ModelComponent(renderer::ModelId());
		}

		return renderer::createModelComponent(
				entt,
				modelCompEntry["index"]
			);
	}

	/// @copydoc saveModelComponent
	void saveModelComponent(
		const renderer::ModelComponent& mc,
		nlohmann::json& componentData
	)
	{
		if (mc.modelId.isNull())
			componentData["index"] = nlohmann::json(nullptr);
		else
			componentData["index"] = mc.modelId.getId();
	}
} // namespace clz::scene
