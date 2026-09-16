/**
 * @file cross_system_flags.hpp
 * @author curl0z
 * @brief Process all flags hinted to editor by other subsystems
 */
#pragma once

#include "core/enginestate.hpp"
#include <cstdint>

namespace clz::editor
{
	void flagEditorStateChange(
		clz::state::EngineState oldState,
		clz::state::EngineState newState);

	void flagEditorFramebufferResize(
		uint32_t width,
		uint32_t height);
}
