#include "../../../include/offscreen/backend/ubo.hpp"
#include "core/logs.hpp"
#include "include/editor_types.hpp"
#include "renderer/arena.hpp"
#include "renderer/pipelinedata/camera.hpp"
#include "renderer/pipelinedata/pipeline_data.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/ubo.hpp"
#include "renderer/utility/offsetalignment.hpp"
#include <string>

namespace clz::editor::backend
{
	bool createEditorUniformBuffers()
	{
		renderer::UniformBufferCreateInfo uboCI{
			.size = sizeof(CameraShaderUBO),
			.bindPoint = renderer::CameraDescriptor.CAMERA_BIND_POINT,
		};
		if (!renderer::createUniformBuffer(
				uboCI,
				editorCameraUBO,
				"editor camera"
		))
		{
			clz::log::error("Could not create editor camera ubo", true);
			return false;
		}
		return true;
	}

	void destroyEditorUniformBuffers()
	{
		renderer::destroyUniformBuffer(editorCameraUBO);
	}
} // namespace clz::editor::backend
