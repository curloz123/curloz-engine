/**
 * @file enginestate.hpp
 * @author curl0z
 * @brief Engine runtime state.
 */

#pragma once

#include "logs.hpp"
#include <string>

namespace clz::state
{
	/**
	 * @brief Describes current state of engine
	 * Game means engine is in game mode
	 * Editor means engine is in sandbox mode (Only Enabled when CLZ_ENABLE_SANDBOX is defined)
	 * Shutdown means, engine has to shut down
	 */
	enum class EngineState
	{
		Game,
		Sandbox,
		Shutdown
	};

	/// @brief Global engine state. The engine will shut down once enum is set to
	/// EngineState::Shutdown
#ifdef CLZ_ENABLE_SANDBOX
	inline EngineState g_engineState = EngineState::Sandbox;
#else
	inline EngineState g_engineState = EngineState::Game;
#endif

	/**
	 * @brief Changes state of engine
	 *
	 * @param state Change state to?
	 * @param callerLocation Location where this function is being called
	 */
	inline void setEngineState(const EngineState state, const std::string_view callerLocation)
	{
		g_engineState = state;
		switch (g_engineState)
		{
		case EngineState::Game:
			clz::log::info("Engine state set to Running by: " + std::string(callerLocation));
			break;

		case EngineState::Sandbox:
			clz::log::info("Engine state set to Editor by: " + std::string(callerLocation));
			break;

		case EngineState::Shutdown:
			clz::log::info("Engine state set to Shutdown by: " + std::string(callerLocation));
			break;
		}
	}

} // namespace clz::state
