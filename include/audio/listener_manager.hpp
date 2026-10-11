/**
 * @file listener_manager.hpp
 * @author curl0z
 * @brief Provides audio listener related functions
 */

#pragma once

#include "native.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/camera/cameradata.hpp"
#include "renderer/renderer.hpp"
#include "math/vec3.hpp"
#include "core/enginestate.hpp"
#include <array>
#ifdef CLZ_ENABLE_EDITOR
#include "../../editor/include/sceneview.hpp"
#endif

namespace clz::audio
{
	/// @brief Updates listener data
	/// @note Internally retrieves camera's data. 
	/// Must be called only after entities have loaded
	inline void updateListenerData()
	{
		/// --- Update listener Data --- ///
		renderer::CameraId cameraId;
#ifdef CLZ_ENABLE_EDITOR
		if (state::g_engineState == state::EngineState::Editor)
		{
			cameraId = editor::getEditorMainViewCameraId();
		}
		else 
#endif
		{
			cameraId = renderer::getGameCameraHandle();

		}
		math::vec3 cameraPos = renderer::getCameraPosition(cameraId);
		math::vec3 front     = renderer::getCameraLocalFrontVector(cameraId);
		math::vec3 up	     = math::cross(renderer::getCameraLocalRightVector(cameraId), front);
		math::vec3 velocity  = renderer::getCameraVelocity(cameraId);

		alListener3f(
			AL_POSITION, 
			cameraPos.x, 
			cameraPos.y,
			cameraPos.z
		);
		alListener3f(
			AL_VELOCITY,
			velocity.x,
			velocity.y,
			velocity.z
		);
		const std::array orientation = {
			front.x, front.y, front.z,
			up.x, up.y, up.z
		};
		alListenerfv(AL_ORIENTATION, orientation.data());

	}
}
