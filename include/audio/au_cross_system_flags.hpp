/**
 * @file au_cross_system_flags.hpp
 * @author curl0z
 * @brief Cross system flags header for audio system
 */

#pragma once

#include "listener.hpp"

namespace clz::audio
{
	/// @brief Hints audio system entities have loaded
	inline void flagAudioSystemEntitiesLoaded()
	{
		updateListenerData();	
	}
}
