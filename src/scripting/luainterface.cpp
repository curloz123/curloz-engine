/**
 * @file luainterface.cpp
 * @author curl0z
 * @brief Lua scripting interface implementation
 */

#include "scripting/luainterface.hpp"
#include "core/logs.hpp"
#include "window/inputmanager.hpp"

namespace clz::script
{
	void bindKeyboardKeys(sol::state& rLua)
	{
		/*
		// ALl keyboard binds
		rLua["Key"] =
		    rLua.create_table_with("Escape", static_cast<int>(clz::window::Key::Escape),
					   "Space", static_cast<int>(clz::window::Key::Space),
					   "Enter", static_cast<int>(clz::window::Key::Enter), "W",
					   static_cast<int>(clz::window::Key::W), "A",
					   static_cast<int>(clz::window::Key::A), "S",
					   static_cast<int>(clz::window::Key::S), "D",
					   static_cast<int>(clz::window::Key::D), "Up",
					   static_cast<int>(clz::window::Key::Up), "Down",
					   static_cast<int>(clz::window::Key::Down), "Left",
					   static_cast<int>(clz::window::Key::Left), "Right",
					   static_cast<int>(clz::window::Key::Right));
		*/
	}

	void log(const std::string& msg)
	{
		// clz::log::info(msg);
	}

	bool isKeyPressed(int key)
	{
		// return clz::window::isPressed(static_cast<clz::window::Key>(key));
	}

	bool isKeyReleased(int key)
	{
		// return clz::window::isReleased(static_cast<clz::window::Key>(key));
	}

} // namespace clz::script