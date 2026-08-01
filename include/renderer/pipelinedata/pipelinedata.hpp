/**
 * @file pipelinedata.hpp
 * @author curl0z
 * @brief Top-level entry points for the main pipeline's per-frame input
 * data: uniform buffers, combined sampler, and their descriptor sets.
 *
 * @note preparePipelineData() must run once, before the pipeline itself is
 * created (it produces the descriptor set layouts the pipeline layout is
 * built from). updatePipelineData() must run once scene textures exist,
 * to write the actual sampler bindings. destroyPipelineData() tears
 * everything down in reverse order.
 */
#pragma once

namespace clz::renderer
{
/// @brief Creates uniform buffers + their layout, the combined sampler
/// layout, and allocates/writes the descriptor sets for both. Call
/// once, before pipeline creation.
/// @return true on success.
bool preparePipelineData();

/// @brief Writes the combined sampler's descriptor set bindings from
/// currently-loaded scene textures. Call once texture data actually
/// exists (i.e. after a scene has finished loading).
void updatePipelineData();

/// @brief Destroys descriptor sets/pool, both descriptor set layouts,
/// and the uniform buffer memory. Call once, on scene unload / shutdown.
void destroyPipelineData();
} // namespace clz::renderer