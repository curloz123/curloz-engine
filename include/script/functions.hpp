/**
 * @file functions.hpp
 * @author curl0z
 * @brief Introduces C++ side functions to LUA
 */

#pragma once

namespace clz::script
{
	/**
	 * @brief Registers core, engine-agnostic native functions into Lua.
	 *
	 * Exposes the `log` table to Lua, with `log.error`, `log.warn`,
	 * `log.debug`, and `log.info` functions that route into the engine's
	 * logging system (clz::log).
	 *
	 * Must be called after #initializeSol() and before any script that
	 * relies on the `log` global is loaded.
	 */
	void registerCoreFunctions();

	/**
	 * @brief Registers entity-related native functions into Lua.
	 *
	 * Exposes the `entity` table to Lua, currently providing
	 * `entity.getEntityByName(name)`, which returns the matching entity
	 * or `ecs::NULL_ENTITY` if no entity with that name exists.
	 *
	 * Must be called after #initializeSol() and before any script that
	 * relies on the `entity` global is loaded.
	 */
	void registerEntityFunctions();
}
