#pragma once

#include "core/enginestate.hpp"
#include "mouse.hpp"
#include "window/window_types.hpp"

namespace clz::window
{
	inline void flagRendererStateChange(
		const clz::state::EngineState oldState,
		const clz::state::EngineState newState)
	{
		double cursorX = 0;
		double cursorY = 0;
		glfwGetCursorPos(w_window, &cursorX, &cursorY);
		cursorPosThisFrame = math::vec2(
			static_cast<float>(cursorX),
			static_cast<float>(cursorY));
		cursorPosLastFrame = math::vec2(
			static_cast<float>(cursorX),
			static_cast<float>(cursorY));
		cursorOffset = math::vec2(0.0f, 0.0f);
	}
}
