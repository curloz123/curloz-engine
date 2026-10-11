/**
 * @file audio_components.hpp
 * @author curl0z
 * @brief Defines all audio components
 */

#pragma once

#include "buffer_manager.hpp"
#include "buffer_player.hpp"

namespace clz::audio
{
	///< @brief Audio buffer player component.
	struct AudioBufferPlayerComponent
	{
		BufferPlayerId bufferPlayerId;
	};

	/// @brief Creates a buffer player component.
	/// @return Newly created buffer component.
	/// @note Check whether the bufferPlayerId is null or not.
	inline AudioBufferPlayerComponent createAudioBufferPlayerComponentForEntity(
		BufferPlayerDef& bufferPlayerDef
	)
	{
		return AudioBufferPlayerComponent{
			.bufferPlayerId = createBufferPlayer(bufferPlayerDef)
		};
	}

	///< @brief Audio buffer component
	struct AudioBufferComponent
	{
		BufferId bufferId;
	};

	/// @brief Creates a buffer component.
	/// @param audioFile Path to audio file.
	/// @return bufferComponent Newly created buffer component.
	/// @note Check whether the bufferId is null or not.
	inline AudioBufferComponent createAudioBufferComponent(const std::filesystem::path& audioFile)
	{
		return AudioBufferComponent{
			.bufferId = loadBuffer(audioFile)
		};
	}
	
}
