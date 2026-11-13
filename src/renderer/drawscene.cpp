/**
 * @file drawscene.cpp
 * @author curl0z
 * @brief Scene's main drawing file
 * A lot of compile time definitions have been used in if-else blocks
 * Because we don't want any editor stuff in main game binary
 */

#include "renderer/drawscene.hpp"
#include "core/enginestate.hpp"
#include "renderer/model/model.hpp"
#include "renderer/pipelinedata/camera.hpp"
#include "renderer/pipelinedata/lights.hpp"
#include "renderer/pipelinedata/pipeline_data.hpp"
#include "renderer/pipelinedata/texture.hpp"
#include "renderer/vk_types.hpp"

#ifdef CLZ_ENABLE_EDITOR
#include "include/offscreen/offscreentarget.hpp"
#include "include/sceneview.hpp"
#include "renderer/camera/camerafunctions.hpp"
#endif

namespace clz::renderer
{
	/// @copydoc lastMainDraw
	void lastMainDraw(VkCommandBuffer commandBuffer)
	{
		CameraId activeCameraId = NULL_CAMERA;
		CameraShaderUBO cameraShaderUBO;

#ifdef CLZ_ENABLE_EDITOR
		if (clz::state::g_engineState == clz::state::EngineState::Editor)
		{
			// editor handles camera updating itself
			activeCameraId = editor::mainViewportImage.getCameraId();
			const math::vec3 camPos = getCameraPosition(activeCameraId);
			auto extent = editor::mainViewportImage.getExtent();
			cameraShaderUBO = {
				.projection = getCameraProjMatrix(
					activeCameraId,
					static_cast<float>(extent.width),
					static_cast<float>(extent.height)
				),
				.view = getCameraViewMatrix(activeCameraId),
				.cameraPos = math::vec4(camPos.x, camPos.y, camPos.z, 1.0f),
			};
		}
		else
#endif
		/// --- This part is an if-else block when editor is enabled ---
		/// --- In main binary, its the only part ---
		{
			/// Rendering camera is handled by renderer itself
			updateCamera(r_cameraId);
			activeCameraId = r_cameraId;
			const math::vec3 camPos = getCameraPosition(activeCameraId);
			cameraShaderUBO = {
				.projection = getCameraProjMatrix(
					activeCameraId,
					static_cast<float>(r_renderTargetContext.imageExtent.width),
					static_cast<float>(r_renderTargetContext.imageExtent.height)
				),
				.view = getCameraViewMatrix(activeCameraId),
				.cameraPos = math::vec4(camPos.x, camPos.y, camPos.z, 1.0f),
			};
		}


		vkCmdBindPipeline(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			r_pipelineContext.pipeline
		);

		// Descriptor sets

		CameraDescriptor.update(cameraShaderUBO);
		LightDescriptor.update();
		CameraDescriptor.bindDescriptor(commandBuffer);
		TextureDescriptor.bindDescriptor(commandBuffer, r_pipelineContext);
		LightDescriptor.bindDescriptor(commandBuffer);

		ModelManager.drawAll(commandBuffer);
	}

} // namespace clz::renderer

namespace clz::renderer
{
	/// @copydoc
	void drawScene(VkCommandBuffer commandBuffer)
	{
		lastMainDraw(commandBuffer);
	}

} // namespace clz::renderer
