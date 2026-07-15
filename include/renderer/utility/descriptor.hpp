#pragma once

#include "ubo.hpp"
#include <vector>
#include "renderer/context/pipelinecontext.hpp"

namespace clz::renderer
{
	bool createDescriptor(PipelineContext& pipelineContext,
		bool enableUBO, const std::vector<UBO*>& pUniformBuffers, UBOMemory& uboMemory,
		bool enableTextureBuffer);
	void destroyDescriptor(PipelineContext& pipelineContext, UBOMemory& uboMemory);

	void updateTextureData(PipelineContext& pipelineContext);
}