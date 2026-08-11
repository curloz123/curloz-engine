/**
 * @file pipelinedata.cpp
 * @author curl0z
 * @brief Implementation of the top-level pipeline input entry points.
 * Order matters here: layouts (UBO, sampler) must exist before
 * createDescriptors() allocates sets from them.
 */

#include "renderer/pipelinedata/pipelinedata.hpp"
#include "core/logs.hpp"
#include "renderer/pipelinedata/camera.hpp"
#include "renderer/pipelinedata/descriptor.hpp"
#include "renderer/pipelinedata/lights.hpp"
#include "renderer/pipelinedata/texture.hpp"

namespace clz::renderer
{
	bool preparePipelineData()
	{
		// 1. Uniform buffers + their descriptor set layout(s).
		if (!initCameraDescriptor())
		{
			clz::log::error("Could not create camera descriptor!");
			return false;
		}

		// 2. Combined image sampler's descriptor set layout.
		if (!createTextureDescriptor())
		{
			clz::log::error("Could not create combined sampler layout!");
			return false;
		}

		if (!initLightDescriptor())
		{
			clz::log::error("Could not create light descriptor!");
			return false;
		}
		// 3. Pool + actual descriptor sets, built from the layouts above.
		//    Camera UBO bindings are written here; sampler bindings are
		//    deferred to updatePipelineData() since no textures exist yet.
		if (!createDescriptorPool())
		{
			clz::log::error("Could not create descriptors pool");
			return false;
		}
		if (!allocateDescriptorSets())
		{
			clz::log::error("Could not allocate descriptor sets");
			return false;
		}

		return true;
	}

	void updatePipelineData()
	{
		// Scene textures now exist; write the sampler descriptor bindings.
		updateTextureForDescriptors(textureDescriptorSets);
	}

	void destroyPipelineData()
	{
		destroyDescriptorPool();
		destroyLightDescriptor();
		destroyTextureDescriptor();
		destroyCameraDescriptor();
	}
} // namespace clz::renderer