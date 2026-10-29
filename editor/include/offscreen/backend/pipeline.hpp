#pragma once

#include "core/logs.hpp"
#include "math/mat4x4.hpp"
#include "renderer/context/context.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/pipelinedata/camera.hpp"
#include "renderer/vk_types.hpp"

namespace clz::editor::backend
{
	inline renderer::PipelineContext editorPipelineContext{};

	struct EditorPushConstant
	{
		math::mat4 model;
		uint32_t materialIndex;
	};
}

namespace clz::editor::backend
{
	bool initializeEditorPipeline();
	void destroyEditorPipeline();
	void updateTextureBufferObject();
} // namespace clz::editor::backend
