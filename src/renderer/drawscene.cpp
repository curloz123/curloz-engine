#include "core/enginestate.hpp"
#include "renderer/drawscene.hpp"
#include "renderer/model/model.hpp"
#include "renderer/pipelinedata/descriptor.hpp"
#include "renderer/pipelinedata/ubo.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/vk_types.hpp"
#include <array>

#ifdef CLZ_ENABLE_EDITOR
#include "include/offscreen/offscreentarget.hpp"
#include "include/sceneview.hpp"
#endif

namespace clz::renderer
{
	void lastMainDraw(const VkCommandBuffer commandBuffer)
	{
		CameraId cameraId = NULL_CAMERA;
		math::mat4 view;
		math::mat4 projection;
#ifdef CLZ_ENABLE_EDITOR
		if (state::g_engineState == state::EngineState::Editor)
		{
			// editor handles camera updating itself
			cameraId = editor::mainViewportImage.cameraId;
			view = getCameraViewMatrix(cameraId);
			projection = getCameraProjMatrix(
				cameraId,
				editor::mainViewportImage.extent.width,
				editor::mainViewportImage.extent.height);
		}
		else
#endif
		{
			useCamera(r_cameraId);
			updateCamera(r_cameraId);
			cameraId = r_cameraId;
			view = getCameraViewMatrix(cameraId);
			projection = getCameraProjMatrix(
				cameraId,
				r_swapchainContext.extent.width,
				r_swapchainContext.extent.height);
		}

		vkCmdBindPipeline(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			r_pipelineContext.pipeline);

		// Descriptor sets

		const CameraShaderUBO cameraShaderUBO = {
			.projection = projection,
			.view = view
		};
		updateUniformBuffers(cameraShaderUBO);
		const std::array descriptorSets = {
			cameraDescriptorSets[r_currentFrame], // binding point is 0
			samplerDescriptorSets[r_currentFrame] // As sampler's binding point is 1
		};
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			r_pipelineContext.layout,
			0,
			descriptorSets.size(),
			descriptorSets.data(),
			0,
			nullptr);

		drawAllModels(commandBuffer);

	}

}

namespace clz::renderer
{
	void drawScene(const VkCommandBuffer commandBuffer)
	{
		lastMainDraw(commandBuffer);
	}

}