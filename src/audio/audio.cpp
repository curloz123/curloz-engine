/**
 * @file audio.cpp
 * @author curl0z
 * @brief Audio system public header implementation
 */

#include "audio/audio.hpp"
#include "audio/audio_components.hpp"
#include "core/logs.hpp"
#include "audio/buffer_manager.hpp"
#include "core/enginestate.hpp"
#include "renderer/renderer.hpp"
#ifdef CLZ_ENABLE_EDITOR
#include "../../editor/include/sceneview.hpp"
#endif


namespace clz::audio
{
	/// @copydoc init
	bool init()
	{
		if (!initializeOpenAL())
		{
			clz::log::error("Could not initialize OpenAL");
			return false;
		}

		/// --- generate all sources and related data --- ///
		generateAllSources();
		

		clz::log::info("Initialized audio system");
		return true;
	}

	/// @copydoc update
	void update()
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


		/// --- Remove all inactive/completed buffer players --- ///
		std::erase_if(
			au_activeBufferPlayers,
			[](BufferPlayerId& bufferPlayerId)
			{
				auto& associatedSourceId = 
					au_associatedSources[bufferPlayerId.getId()];	
				if (!isSourceBusy(associatedSourceId))
				{
					freeSource(associatedSourceId);
					associatedSourceId.nullify();
					return true;
				}
				return false;
			}
		);
		/// --- Update all 'Active' buffer players --- ///
		for (const auto& bufferPlayerId : au_activeBufferPlayers)
		{
			updateBufferPlayerData(bufferPlayerId);
		}
	}

	/// @copydoc shutdown
	void shutdown()
	{
		/// --- delete all buffers --- ///
		deleteAllBuffers();

		closeOpenAL();
		clz::log::info("Shut down audio system");
	}
} // namespace clz::audio
