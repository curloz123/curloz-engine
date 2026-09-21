/**
 * @file script.hpp
 * @author curl0z
 * @brief Script system public header
 */

#pragma once

#include <filesystem>

namespace clz::script
{
	/**
	 * @brief Initializes the scripting system.
	 *
	 * Initializes the underlying sol2/Lua state and registers all
	 * native (C++) functions exposed to Lua, including core functions
	 * (e.g. `log`) and entity-related functions (e.g. `entity.getEntityByName`).
	 *
	 * @return true if the scripting system was initialized successfully,
	 *         false if sol2 initialization failed.
	 */
	bool init();

	/**
	 * @brief Shuts down the scripting system.
	 *
	 * Releases any resources owned by the scripting system. Currently a
	 * no-op placeholder.
	 */
	void shutdown();
}
