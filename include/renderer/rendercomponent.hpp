/**
 * @file rendercomponent.hpp
 * @author curl0z
 *
 * @brief Wraps all functions regarding entity-components
 * that are related to renderer
 */
#pragma once

#include "core/logs.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "renderer/entitydata/uvbuffer.hpp"
#include "renderer/entitydata/texture.hpp"
#include "entitydata/modeldata.hpp"
#include "pipelineinput/mainpipeline.hpp"
#include "renderer/context/pipelinecontext.hpp"
#include "scene/entity/components.hpp"
#include "vk_types.hpp"
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
		VBuffer::submitVertexBuffer();
		IBuffer::submitIndexBuffer();
		UVBuffer::submitUVBuffer();
		createTextures();

		clz::log::info("Updating pipeline input, after entities have loaded");
		updateTextureData(r_pipelineContext);
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
		const ModelID modelID = Asset::loadModel(path);
		ecs::ModelComponent modelComponent(modelID);
		return modelComponent;
	}
} // namespace clz::renderer