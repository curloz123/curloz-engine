/**
 * @file au_cross_system_flags.hpp
 * @author curl0z
 * @brief Cross system flags header for audio system
 */

#pragma once

#include "listener_manager.hpp"
#include "buffer_player.hpp"
#include "core/enginestate.hpp"

namespace clz::audio
{
	/// @brief Hints audio system entities have loaded
	inline void flagAudioSystemEntitiesLoaded()
	{
		updateListenerData();	
	}

	/// @brief Hints audio system system has changed.
	/// Internally stops all playing buffers
	/// @warn is not initiated right now
	inline void hintAudioSystemStateChanged(
		const state::EngineState oldState,
		const state::EngineState newState
	)
	{
		return;
		if (newState == state::EngineState::Editor &&
			oldState == state::EngineState::Game)
		{
			bufferPlayerStopAll();	
		}
	}
}
