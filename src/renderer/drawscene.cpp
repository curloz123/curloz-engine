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
#include "renderer/pipelinedata/descriptor.hpp"
#include "renderer/pipelinedata/lights.hpp"
#include "renderer/pipelinedata/texture.hpp"
#include "renderer/vk_types.hpp"

#include <array>

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
		math::mat4 view;
		math::mat4 projection;
#ifdef CLZ_ENABLE_EDITOR
		if (state::g_engineState == state::EngineState::Editor)
		{
			// editor handles camera updating itself
			activeCameraId = editor::mainViewportImage.cameraId;
			view = getCameraViewMatrix(activeCameraId);
			projection = getCameraProjMatrix(
				activeCameraId,
				static_cast<float>(editor::mainViewportImage.extent.width),
				static_cast<float>(editor::mainViewportImage.extent.height)
			);
		}
		else
#endif
		/// --- This part is an if-else block when editor is enabled ---
		/// --- In main binary, its the only part ---
		{
			/// Rendering camera is handled by renderer itself
			useCamera(r_cameraId);
			updateCamera(r_cameraId);
			activeCameraId = r_cameraId;
			view = getCameraViewMatrix(activeCameraId);
			projection = getCameraProjMatrix(
				activeCameraId,
				(float)r_swapchainContext.extent.width,
				(float)r_swapchainContext.extent.height
			);
		}
#ifdef CLZ_ENABLE_EDITOR
		if (state::g_engineState == state::EngineState::Game)
		{
			const auto editorCameraId = editor::mainViewportImage.cameraId;

			setCameraPosition(editorCameraId, getCameraPosition(r_cameraId));
			setCameraPitch(editorCameraId, getCameraPitch(r_cameraId));
			setCameraYaw(editorCameraId, getCameraYaw(r_cameraId));

			updateCameraVectors(editorCameraId);
		}
#endif

		vkCmdBindPipeline(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			r_pipelineContext.pipeline
		);

		// Descriptor sets

		const math::vec3 camPos = getCameraPosition(activeCameraId);
		const CameraShaderUBO cameraShaderUBO = {
			.projection = projection,
			.view = view,
			.cameraPos = math::vec4(camPos.x, camPos.y, camPos.z, 1.0f),
		};
		updateCameraDescriptor(cameraShaderUBO);
		updateLightDescriptor();
		std::array<VkDescriptorSet, 3> descriptorSets = {};
		descriptorSets[CAMERA_SET_POINT] = cameraDescriptorSets[r_currentFrame];
		descriptorSets[TEXTURE_SET_POINT] = textureDescriptorSets[r_currentFrame];
		descriptorSets[LIGHT_SET_POINT] = lightDescriptorSets[r_currentFrame];

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			r_pipelineContext.layout,
			0,
			descriptorSets.size(),
			descriptorSets.data(),
			0,
			nullptr
		);

		drawAllModels(commandBuffer);
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