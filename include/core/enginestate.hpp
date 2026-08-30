/**
 * @file enginestate.hpp
 * @author curl0z
 * @brief Engine runtime state.
 */

#pragma once

#include "string_view"

#ifdef CLZ_ENABLE_EDITOR
#include "include/editor.hpp"
#include "window/inputmanager.hpp"
#include "window/mouse.hpp"
#endif

namespace clz::state
{
	/**
	 * @brief Describes current state of engine
	 * Game means engine is in game mode
	 * Editor means engine is in edit mode (Only Enabled when CLZ_ENABLE_EDITOR is
	 * defined) Shutdown means, engine has to shut down
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
	void setEngineState(
		EngineState state,
		std::string_view callerLocation);

	/**
	 * @brief Checks per frame if engine state has to be updated or not
	 * Currently only used to check whether to shift bw editor and game mode
	 */
	void updateEngineState();

} // namespace clz::state
