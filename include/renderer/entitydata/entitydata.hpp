#pragma once

/**
 * @file entitydata.hpp
 * @author curl0z
 * @brief Contains function related to entity data
 */

#include "core/logs.hpp"
#include "renderer/entitydata/entitydata.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/entitydata/uvbuffer.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"

namespace clz::renderer
{
/// @brief Prepares all entity data (vertices, indices, UV, textures etc...)
inline bool prepareEntityData()
{
	if (!createVertexBuffer())
	{
		clz::log::error("Failed to create vertex buffer");
		return false;
	}
	if (!createIndexBuffer())
	{
		clz::log::error("Failed to create index buffer");
		return false;
	}
	if (!createUVBuffer())
	{
		clz::log::error("Failed to create UV buffer");
		return false;
	}
	if (!createTextures())
	{
		clz::log::error("Failed to create textures");
		return false;
	}

	clz::log::info("Created entity data in renderer");
	return true;
}

/// @brief Destroys all stored entity data in memory
inline void destroyEntityData()
{
	destroyTextures();
	destroyUVBuffer();
	destroyIndexBuffer();
	destroyVertexBuffer();

	clz::log::info("Destroyed entity data");
}
} // namespace clz::renderer