/**
 * @file pipelinedata.cpp
 * @author curl0z
 * @brief Implementation of the top-level pipeline input entry points.
 * Order matters here: layouts (UBO, sampler) must exist before
 * createDescriptors() allocates sets from them.
 */

#include "renderer/pipelinedata/pipelinedata.hpp"
#include "core/logs.hpp"
#include "renderer/pipelinedata/descriptor.hpp"
#include "renderer/pipelinedata/sampler.hpp"
#include "renderer/pipelinedata/ubo.hpp"

namespace clz::renderer
{
bool preparePipelineData()
{
	// 1. Uniform buffers + their descriptor set layout(s).
	if (!createUniformBuffers())
	{
		clz::log::error("Could not create uniform buffers!");
		return false;
	}

	// 2. Combined image sampler's descriptor set layout.
	if (!createCombinedSamplersLayout())
	{
		clz::log::error("Could not create combined sampler layout!");
		return false;
	}

	// 3. Pool + actual descriptor sets, built from the layouts above.
	//    Camera UBO bindings are written here; sampler bindings are
	//    deferred to updatePipelineData() since no textures exist yet.
	if (!createDescriptors())
	{
		clz::log::error("Could not create descriptors!");
		return false;
	}

	return true;
}

void updatePipelineData()
{
	// Scene textures now exist; write the sampler descriptor bindings.
	updateSamplersDataForDescriptorSets(samplerDescriptorSets);
}

void destroyPipelineData()
{
	// Reverse order: descriptor sets/pool first (they reference the
	// layouts and the buffer), then layouts, then the buffer memory itself.
	destroyDescriptors();
	destroyCombinedImageSamplersLayout();
	destroyUniformBuffers();
}
} // namespace clz::renderer