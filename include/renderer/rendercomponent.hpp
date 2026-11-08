/**
 * @file rendercomponent.hpp
 * @author curl0z
 *
 * @brief Wraps all functions regarding entity-components
 * that are related to renderer
 */
#pragma once

#include "core/logs.hpp"
#include "entity/entitymanager.hpp"
#include "lighting/lighting.hpp"
#include "model/model.hpp"
#include "model/model_matrix.hpp"
#include <expected>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	/**
	 * @brief This function kind of acts like a flag,
	 * that is set whenever the ecs subsystem has finished
	 * loading render components of all entities
	 *
	 * Initializes entity data, which further allocates
	 * memory and relevant stuff internally
	 */
	inline void flagRenderComponentsLoaded()
	{
	}

	/**
	 * @brief Stores a renderable ModelID, that can be used to draw models
	 * Stores model's Id which references the actual model
	 * and matricesRange which is a handle to all matrices this entity's model's nodes have.
	 */
	struct ModelComponent
	{
		ModelId modelId;
		ModelMatrixRange matricesRange;
	};

	struct DirectionalLightComponent
	{
		DirectionalLightId Id;
	};
	struct PointLightComponent
	{
		PointLightId Id;
	};
	struct SpotLightComponent
	{
		SpotLightId Id;
	};


	/// @brief Creates model component.
	/// Internally registers entity for this model, and allocates matrix range to it.
	/// @param entt The entity for which we're creating this component.
	/// @param modelIndex The actual model index stored in JSON's data.
	/// @return ModelComponent on successful creation, error string otherwise.
	inline std::expected<ModelComponent, std::string>
	createModelComponent(
		const ecs::entity entt,
		const uint32_t modelIndex
	)
	{
		ModelId modelId;
		ModelMatrixRange range;

		if (ModelManager.attachModelToEntity(entt, modelIndex)) [[likely]]
		{
			modelId = ModelId(modelIndex);
		}
		else
		{
			const std::string error =
				"could not retrieve model Id for entity: " +
				ecs::getEntityName(entt);

			clz::log::error(error);
			return std::unexpected(error);
		}


		auto& matrixManager = ModelManager.getMatrixManager();
		auto matrixCount =
			ModelManager.getMatrixCount(modelId);
		if (!matrixCount) [[unlikely]]
		{
			const std::string error =
				"Unable to retrieve matrix range for entity: " +
				ecs::getEntityName(entt);

			clz::log::error(error);
			return std::unexpected(error);
		}

		if (!matrixManager.allocateRange(
			matrixCount.value(),
			range
		)) [[unlikely]]
		{
			const std::string error =
				"Unable to retrieve matrix range for entity: " +
				ecs::getEntityName(entt);

			clz::log::error(error);
			return std::unexpected(error);
		}

		CameraDescriptor.updateMDescriptorSets();

		ModelComponent modelComp;
		modelComp.modelId = modelId;
		modelComp.matricesRange = range;

		return modelComp;
	}

} // namespace clz::renderer
