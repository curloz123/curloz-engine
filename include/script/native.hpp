/**
 * @file native.hpp
 * @author curl0z
 * @brief SOL's(third_party) include header
 */

#pragma once

#include <sol/sol.hpp>

namespace clz::script
{
	/**
	 * @brief The engine's single, global sol2/Lua state.
	 *
	 * All script loading, native function registration, and Lua execution
	 * (sensor scripts, collision scripts, core/entity bindings) goes
	 * through this shared state.
	 */
	inline sol::state s_SolHandle;

	/**
	 * @brief Opens the required Lua standard libraries on #s_SolHandle.
	 *
	 * Currently opens `base`, `math`, and `os`. This must be called before
	 * any script is loaded or any native function is registered.
	 *
	 * @return true if the state was initialized successfully.
	 */
	bool initializeSol();
}
