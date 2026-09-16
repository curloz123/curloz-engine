/**
 * @file core.hpp
 * @author curl0z
 * @brief General header for including all core headers.
 */

#pragma once

#include "assert.hpp"
#include "enginestate.hpp"
#include "kb_shortcuts.hpp"
#include "logs.hpp"
#include "time.hpp"

namespace clz
{
	/// @brief Updates all core systems.
	/// @note computeTime must be the FIRST first function to be called
	/// inside this function.
	inline void updateCoreSystems()
	{
		time::computeTime();
		state::updateEngineState();
		processShortcuts();
	}
}
