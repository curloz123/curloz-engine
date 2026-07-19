/**
 * @file rendercomponent.hpp
 * @author curl0z
 *
 * @brief Wraps all functions regarding entity-components
 * that are related to renderer
 */
#pragma once

#include "core/logs.hpp"
#include "model/model.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/entitydata/uvbuffer.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "renderer/pipelinedata/pipelinedata.hpp"
#include "scene/entity/components.hpp"
#include <filesystem>

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
		createVertexBuffer();
		createIndexBuffer();
		createUVBuffer();
		createTextures();

		updatePipelineData();
		clz::log::info("Updated pipeline input, after entities have loaded");
	}

	/**
	 *
	 * @param path Path to the model file
	 *
	 * @brief Tells renderer to create(register) a model,
	 * calls Asset::loadModel which returns handle referencing to the model.
	 *
	 * @return ecs::ModelComponent which internally references a model
	 */
	inline ecs::ModelComponent createModelComponent(const std::filesystem::path& path)
	{
		if (const auto result = loadModel(path))
		{
			return ecs::ModelComponent(result.value());
		}

		log::warn("Could not load model, exiting");
		return ecs::ModelComponent(NULL_MODEL);
	}
} // namespace clz::renderer