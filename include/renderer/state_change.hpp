#pragma once

#include "vk_types.hpp"
#include "renderer/camera/camera.hpp"
#include "core/enginestate.hpp"

namespace clz::renderer
{
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
}