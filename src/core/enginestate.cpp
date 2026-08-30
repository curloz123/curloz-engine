#include "core/enginestate.hpp"
#include "include/cross_system_flags.hpp"
#include "renderer/cross_system_flags.hpp"
#include "core/logs.hpp"



namespace clz::state
{
	/// @copydoc setEngineState
	void setEngineState(
		const EngineState state,
		const std::string_view callerLocation)
	{
		if (g_engineState == state)
			return;

		const auto oldState = g_engineState;
		g_engineState = state;
		switch (g_engineState)
		{
		case EngineState::Game:
			clz::log::info(
				"Engine state set to Running by: " + std::string(callerLocation)
			);
			break;

#ifdef CLZ_ENABLE_EDITOR
		case EngineState::Editor:
			clz::log::info(
				"Engine state set to Editor by: " + std::string(callerLocation)
			);
			break;
#endif

		case EngineState::Shutdown:
			clz::log::info(
				"Engine state set to Shutdown by: " + std::string(callerLocation)
			);
			break;
		default:
			clz::log::error(
				"Unknown engine state called by: " + std::string(callerLocation)
			);
		}

		clz::renderer::flagRendererStateChange(oldState, g_engineState);
		clz::editor::flagEditorStateChange(oldState, g_engineState);
	}

	/// @copydoc updateEngineState
	void updateEngineState()
	{
#ifdef CLZ_ENABLE_EDITOR
		/// @brief Hit Ctrl+E in game mode, to exit to edit mode
		if (g_engineState == EngineState::Game &&
			window::isKeyPressed(clz::input::Key::LeftControl) &&
			window::isKeyPressed(clz::input::Key::E))
		{
			setEngineState(EngineState::Editor, "not required to know");
		}

		/// @brief Hit Ctrl+G in edit mode, to enter game mode
		if (g_engineState == EngineState::Editor &&
			window::isKeyPressed(clz::input::Key::LeftControl) &&
			window::isKeyPressed(clz::input::Key::G))
		{
			setEngineState(EngineState::Game, "not required to know");
		}
#endif
	}

}
