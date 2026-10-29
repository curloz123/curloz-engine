/**
 * @file entitydata.hpp
 * @author curl0z
 * @brief Contains function related to entity data
 */

#pragma once

#include "core/logs.hpp"
#include "renderer/entitydata/texture.hpp"
#include "buffer.hpp"

namespace clz::renderer
{
	/// @brief Prepares all entity data (vertices, indices, UV, textures etc...)
	inline bool prepareEntityData()
	{
		if (!StaticBufferManager.initialize(190 * 1024 * 1024))
		{
			clz::log::error("Failed to create vertex buffer");
			return false;
		}
		if (!StaticIndexManager.initialize(140 * 1024 * 1024))
		{
			clz::log::error("Failed to create index buffer");
			return false;
		}

		TextureManager.initialize();

		clz::log::info("Created entity data in renderer");
		return true;
	}

	/// @brief Destroys all stored entity data in memory
	inline void destroyEntityData()
	{
		TextureManager.destroyAllTextureSamplers();
		TextureManager.destroyAllTextures();
		StaticIndexManager.destroy();
		StaticBufferManager.destroy();

		clz::log::info("Destroyed entity data");
	}
} // namespace clz::renderer
