#include "../include/cross_system_flags.hpp"
#include "renderer/camera/camerafunctions.hpp"
#include "../include/sceneview.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/camera/camera.hpp"

namespace clz::editor
{
	void flagEditorStateChange(
		const clz::state::EngineState oldState,
		const clz::state::EngineState newState)
	{
		if (oldState == clz::state::EngineState::Game &&
			newState == clz::state::EngineState::Editor)
		{
			prepareEditor();
			window::enableCursor();

			const auto editorCameraId =mainViewportImage.cameraId;

			renderer::setCameraPosition(editorCameraId, renderer::getCameraPosition(renderer::r_cameraId));
			renderer::setCameraPitch(editorCameraId, renderer::getCameraPitch(renderer::r_cameraId));
			renderer::setCameraYaw(editorCameraId, renderer::getCameraYaw(renderer::r_cameraId));

			renderer::updateCameraVectors(editorCameraId);
		}
	}

	void flagEditorFramebufferResize()
	{
		mainViewportImage.outDated = true;
		physicsBodyShapeImage.outDated = true;
	}
}