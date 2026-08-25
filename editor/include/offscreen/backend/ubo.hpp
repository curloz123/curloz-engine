#pragma once

#include "renderer/pipelinedata/camera.hpp"

namespace clz::editor::backend
{

inline renderer::UBOMemory uboMemory{};

inline constexpr uint8_t NUM_BACKEND_UNIFORM_BUFFERS = 1;

struct CameraShaderUBO
{
	math::mat4 projection;
	math::mat4 view;
};
inline renderer::UBO
	editorCameraUBO(sizeof(CameraShaderUBO), renderer::CAMERA_BIND_POINT);

} // namespace clz::editor::backend

namespace clz::editor::backend
{

bool createEditorUniformBuffers();
void destroyEditorUniformBuffers();

} // namespace clz::editor::backend