#pragma once

#include "core/enginestate.hpp"
#include <cstdint>

namespace clz::editor
{
	void flagEditorStateChange(
		clz::state::EngineState oldState,
		clz::state::EngineState newState);

	void flagEditorFramebufferResize(const uint32_t width, const uint32_t height);

}
