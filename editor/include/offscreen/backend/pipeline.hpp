#pragma once

#include "core/logs.hpp"
#include "math/mat4x4.hpp"
#include "renderer/context/context.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/pipelinedata/ubo.hpp"
#include "renderer/vk_types.hpp"

namespace clz::editor::backend
{
inline renderer::PipelineContext editorPipelineContext{};

}

namespace clz::editor::backend
{
bool initializeEditorPipeline();
void destroyEditorPipeline();
void updateTextureBufferObject();
} // namespace clz::editor::backend