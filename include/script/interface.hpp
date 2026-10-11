/**
 * @file interface.hpp
 * @author curl0z
 * @brief Introduces C++ side interfaces like functions and types to LUA
 */

#pragma once

namespace clz::script
{
	/**
	 * @brief Registers core, engine-agnostic native stuff to Lua.
	 * Like logging as of now.
	 */
	void registerCoreInterface();

	/**
	 * @brief Registers entity system interface into Lua.
	 */
	void registerEntityInterface();


	/// @brief Registers all audio interface to lua.
	void registerAudioInterface();
}
