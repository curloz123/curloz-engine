#pragma once

#include "core/enginestate.hpp"

namespace clz::editor
{
	void flagEditorStateChange(
		clz::state::EngineState oldState,
		clz::state::EngineState newState);


	void flagEditorFramebufferResize();

}
