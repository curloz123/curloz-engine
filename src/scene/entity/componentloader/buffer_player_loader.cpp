/**
 * @file buffer_player_loader.cpp
 * @author curl0z
 * @brief Retrieves and saves back buffer player component
 */

#include "audio/audio_components.hpp"
#include "scene/entity/loader.hpp"
#include "audio/buffer_player.hpp"

namespace clz::scene
{
	/// @copydoc retrieveBufferPlayerComponent
	audio::AudioBufferPlayerComponent retrieveBufferPlayerComponent(
		const nlohmann::json& bufferPlayerTable,
		const ecs::entity e
	)
	{
		audio::BufferPlayerDef playerDef;	

		if (bufferPlayerTable.contains("pitch"))
		{
			playerDef.pitch = 
				bufferPlayerTable["pitch"];
		}
		else
			playerDef.pitch = 1.0f;

		if (bufferPlayerTable.contains("gain"))
		{
			playerDef.gain = 
				bufferPlayerTable["gain"];
		}
		else
			playerDef.gain = 1.0f;

		if (bufferPlayerTable.contains("looping"))
		{
			playerDef.looping = 
				bufferPlayerTable["looping"];
		}
		else
			playerDef.looping = false;

		playerDef.entt = e;
		return audio::createAudioBufferPlayerComponentForEntity(playerDef);
	}

	/// @copydoc saveBufferPlayerComponent
	void saveBufferPlayerComponent(
		const audio::AudioBufferPlayerComponent& bufferPlayerComponent,
		nlohmann::json& bufferPlayerTable
	)
	{
		const auto& playerId = bufferPlayerComponent.bufferPlayerId;

		bufferPlayerTable["gain"] = 
			audio::bufferPlayerGetGain(playerId);
		bufferPlayerTable["pitch"] = 
			audio::bufferPlayerGetPitch(playerId);
		bufferPlayerTable["looping"] = 
			audio::bufferPlayerGetLooping(playerId);
	}

}
