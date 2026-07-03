/**
 * @file scripting.cpp
 * @author curl0z
 * @brief Lua scripting subsystem implementation.
 */

#include "scripting/scripting.hpp"
#include "core/logs.hpp"
#include "scripting/luainterface.hpp"

namespace clz::script
{
	/// @brief Global Lua state.
	static sol::state sc_lua;

	void init()
	{
		// load standard Lua libraries
		sc_lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);

		registerLuaInterface(sc_lua);

		clz::log::info("Scripting subsystem initialized");
	}

	void registerLuaInterface(sol::state& rLua)
	{
		bindKeyboardKeys(rLua);

		rLua.set_function("log", log);
		rLua.set_function("isKeyPressed", isKeyPressed);
		rLua.set_function("isKeyReleased", isKeyReleased);
	}

	void runScript(const std::string& path)
	{
		auto result = sc_lua.script_file(path);
		if (!result.valid())
		{
			sol::error err = result;
			clz::log::error("Script error: " + std::string(err.what()));
		}
	}

} // namespace clz::script