/**
 * @file functions.cpp
 * @author curl0z
 * @brief Introduces C++ side functions to LUA
 */

#include "script/functions.hpp"
#include "core/logs.hpp"
#include "script/native.hpp"
#include "entity/entitymanager.hpp"

namespace clz::script
{
	/// @copydoc registerCoreFunctions
	void registerCoreFunctions()
	{
		/// --- Log functions --- ///
		sol::table log = s_SolHandle.create_table();
		log.set_function(
			"error",
			[](const std::string& err){
				clz::log::error(err);
			}
		);
		log.set_function(
			"warn",
			[](const std::string& warn){
				clz::log::warn(warn);
			}
		);
		log.set_function(
			"debug",
			[](const std::string& dbg){
				clz::log::debug(dbg);
			}
		);
		log.set_function(
			"info",
			[](const std::string& info){
				clz::log::info(info);
			}
		);

		s_SolHandle["log"] = log;
	}

	/// @copydoc registerEntityFunctions
	void registerEntityFunctions()
	{
		/// --- get entity by name --- ///
		sol::table entt = s_SolHandle.create_table();
		entt.set_function(
			"getEntityByName",
			[](const std::string& name){
				const auto result = ecs::getEntityByName(name);
				if (result)
				{
					return result.value();
				}
				clz::log::warn(
					"Script tried retrieve entity with name: '" + 
					name + 
					"' But that entity doesn't exist");
				return ecs::NULL_ENTITY;
			}
		);
		s_SolHandle["entity"] = entt;

	}
}
