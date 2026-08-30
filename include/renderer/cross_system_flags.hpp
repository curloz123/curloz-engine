/**
 * @file cross_system_flags.hpp
 * @author curl0z
 * @brief render subsystem's cross system flags to pass around
 */
#pragma once

#include "renderer.hpp"
#include "core/logs.hpp"
#include "window/mouse.hpp"
#include "core/enginestate.hpp"

namespace clz::renderer
{
	/**
	 * @brief Flags renderer on state change
	 * As of now, just disabled cursor on entering game mode
	 * @param oldState old state of engine
	 * @param newState new state of engine
	 */
	inline void flagRendererStateChange(
		const clz::state::EngineState oldState,
		const clz::state::EngineState newState)
	{
		clz::log::debug("state change called by renderer");
		if (newState == clz::state::EngineState::Game)
		{
			window::disableCursor();
		}
	}

#ifdef CLZ_ENABLE_EDITOR
	inline void flagRendererEditorInitialized()
	{

	}
#endif

	/// @brief Flags renderer about framebuffer resize
	/// As of now, recreates all the renderer's images
	inline void flagRendererFramebufferResize()
	{
		recreateImagesOnFramebufferResize();
	}
}
