/**
 * @file enginestate.hpp
 * @author curl0z
 * @brief Engine runtime state.
 */

#pragma once

#include "assert.hpp"
#include "logs.hpp"
#include "window/inputmanager.hpp"
#include <string>

#ifdef CLZ_ENABLE_EDITOR
#include "include/editor.hpp"
#endif

namespace clz::state
{
/**
 * @brief Describes current state of engine
 * Game means engine is in game mode
 * Editor means engine is in edit mode (Only Enabled when CLZ_ENABLE_EDITOR is defined)
 * Shutdown means, engine has to shut down
 */
enum class EngineState
{
	Game,
	Editor,
	Shutdown
};

/// @brief Global engine state. The engine will shut down once enum is set to
/// EngineState::Shutdown
#ifdef CLZ_ENABLE_EDITOR
inline EngineState g_engineState = EngineState::Editor;
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
	if (g_engineState == state)
		return;

	g_engineState = state;
	switch (g_engineState)
	{
	case EngineState::Game:
		clz::log::info(
			"Engine state set to Running by: " + std::string(callerLocation)
		);
		return;

#ifdef CLZ_ENABLE_EDITOR
	case EngineState::Editor:
		clz::log::info(
			"Engine state set to Editor by: " + std::string(callerLocation)
		);
		return;
#endif

	case EngineState::Shutdown:
		clz::log::info(
			"Engine state set to Shutdown by: " + std::string(callerLocation)
		);
		return;
	default:
		CLZ_ASSERT(
			false,
			"Unknown engine state called by: " + std::string(callerLocation)
		);
	}
}

inline void updateEngineState()
{
#ifdef CLZ_ENABLE_EDITOR
	if (g_engineState == EngineState::Game)
	{
		if (window::isKeyPressed(clz::input::Key::LeftControl) &&
		    window::isKeyPressed(clz::input::Key::E))
		{
			g_engineState = EngineState::Editor;
			editor::prepareEditor();
		}
	}
	if (g_engineState == EngineState::Editor)
	{
		if (window::isKeyPressed(clz::input::Key::LeftControl) &&
		    window::isKeyPressed(clz::input::Key::G))
		{
			g_engineState = EngineState::Game;
		}
	}
#endif
}

} // namespace clz::state
