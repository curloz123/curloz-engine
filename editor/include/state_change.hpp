#pragma once

#include "sceneview.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/camera/camera.hpp"
#include "core/enginestate.hpp"
#include "renderer/camera/camerafunctions.hpp"

namespace clz::editor
{
	inline void flagEditorStateChange(
		const clz::state::EngineState oldState,
		const clz::state::EngineState newState)
	{
		if (oldState == clz::state::EngineState::Game &&
			newState == clz::state::EngineState::Editor)
		{
			prepareEditor();
			window::enableCursor();

			const auto editorCameraId = editor::mainViewportImage.cameraId;

			renderer::setCameraPosition(editorCameraId, renderer::getCameraPosition(renderer::r_cameraId));
			renderer::setCameraPitch(editorCameraId, renderer::getCameraPitch(renderer::r_cameraId));
			renderer::setCameraYaw(editorCameraId, renderer::getCameraYaw(renderer::r_cameraId));

			renderer::updateCameraVectors(editorCameraId);
		}
	}
}
