/**
 * @file physics_scripts.hpp
 * @author curl0z
 * @brief This file introduces all physics side functions that would be triggered
 * by the physics system. Each trigger introduces an event
 */
#include <sol/forward.hpp>
#include <sol/sol.hpp>
#include "entity/entitymanager.hpp"
#include "core/logs.hpp"
#include "native.hpp"

namespace clz::script
{
	/**
	 * @brief A Lua-backed script bound to a physics sensor (trigger volume).
	 *
	 * Wraps a Lua module loaded from a `.lua` file that returns a table
	 * with `onSensorEnter(otherEntity)` and `onSensorExit(otherEntity)`
	 * functions, and an optional `onInit()` function. Instances are
	 * owned and pooled by ScriptComponentInterface<SensorScript>.
	 */
	struct SensorScript
	{
	private:
		/// The Lua `onInit` function, if the script provided one.
		sol::function onInitSolFunction;
		/// The Lua `onSensorEnter` function.
		sol::function onEnterSolFunction;	
		/// The Lua `onSensorExit` function.
		sol::function onExitSolFunction;
	public:
		/**
		 * @brief Calls the script's `onInit` function, if present and valid.
		 *
		 * Intended to be called once, after all entities in the scene have
		 * been loaded (see ScriptComponentInterface::initializeAllScripts).
		 */
		void callInitFunction() const
		{
			sol::protected_function_result result = onInitSolFunction();
			if (!result.valid())
			{
			    sol::error err = result;
			    clz::log::error(std::string("onInit runtime error: ") + err.what());
			}
		}

		/**
		 * @brief Calls the script's `onSensorEnter` function.
		 * @param otherEntity The entity that entered the sensor volume.
		 */
		void triggerOnSensorEnter(const ecs::entity otherEntity) const
		{
			if (onEnterSolFunction.valid()) [[likely]]
				onEnterSolFunction(otherEntity);
		}

		/**
		 * @brief Calls the script's `onSensorExit` function.
		 * @param otherEntity The entity that exited the sensor volume.
		 */
		void triggerOnSensorExit(const ecs::entity otherEntity) const
		{
			if (onExitSolFunction.valid()) [[likely]]
				onExitSolFunction(otherEntity);
		}

		/**
		 * @brief Loads (or reloads) this sensor script from a Lua file.
		 *
		 * Parses and executes @p scriptPath, expecting it to return a Lua
		 * table containing `onSensorEnter` and `onSensorExit` functions,
		 * and optionally an `onInit` function. On success, the resolved
		 * Lua functions replace this instance's bound functions; on
		 * failure, this instance's previously-bound functions (if any)
		 * are left untouched.
		 *
		 * @param scriptPath Path to the `.lua` file to load.
		 * @return true if the script was loaded and validated successfully,
		 *         false on a syntax error, runtime error, or if the
		 *         returned table is missing/malformed.
		 */
		bool loadScript(const std::filesystem::path& scriptPath)
		{
			sol::load_result loadResult = s_SolHandle.load_file(scriptPath);
			if (!loadResult.valid())
			{
				sol::error err = loadResult;
				clz::log::error(
					"SYNTAX error in sensor sript: " +
					scriptPath.string() + ": " + 
					err.what());
				return false;

			}
			sol::protected_function script = loadResult;
			sol::protected_function_result result = script();
			if (!result.valid())
			{
				sol::error err = result;
				clz::log::error("RUNTIME error in sensor sript: " +
						scriptPath.string() + " " + 
						err.what());
				return false;
			}

			sol::object returned = result;
			if (returned.get_type() != sol::type::table)
			{
				clz::log::error("Sensor Script: " + scriptPath.string() + 
						" does not return a table");
				return false;
			}

			sol::table returnedTable = returned;
			sol::object beginFunc = returnedTable["onSensorEnter"];
			sol::object endFunc   = returnedTable["onSensorExit"];

			if (beginFunc.valid())
			{
				if (beginFunc.get_type() == sol::type::function)
				{
					onEnterSolFunction = beginFunc;
				}
				else
				{
					clz::log::error(
						"Sensor Script: " + scriptPath.string() + 
						" has onSensorBegin"
						" but its not a function");
				}
			}
			else
			{
				clz::log::warn("Sensor Script: " + scriptPath.string() + 
						" is missing onSensorBegin"
						" function");
			}
			if (endFunc.valid())
			{
				if (endFunc.get_type() == sol::type::function)
				{
					onExitSolFunction = endFunc;
				}
				else
				{
					clz::log::error(
						"Sensor Script: " + scriptPath.string() + 
						" has onSensorEnd"
						" but its not a function");
				}
			}
			else
			{
				clz::log::warn("Sensor Script: " + scriptPath.string() + 
						" is missing onSensorEnd"
						" function");
			}

			/// --- Init function --- ///
			sol::object initObj = returnedTable["onInit"];
			if (initObj.valid() && initObj.get_type() == sol::type::function)
			{
				onInitSolFunction = initObj;
			}
			else
			{
				clz::log::warn("Sensor script: " + 
						scriptPath.string() + 
						" has no 'onInit' function");
			}

			return true;
		}
	};

	/**
	 * @brief A Lua-backed script bound to a physics collision event.
	 *
	 * Wraps a Lua module loaded from a `.lua` file that returns a table
	 * with an `onCollision(otherEntity)` function, and an optional
	 * `onInit()` function. Instances are owned and pooled by
	 * ScriptComponentInterface<CollisionScript>.
	 */
	struct CollisionScript
	{
	private:
		/// The Lua `onInit` function, if the script provided one.
		sol::function onInitSolFunction;
		/// The Lua `onCollision` function.
		sol::function onCollisionSolFunction;	
	public:
		/**
		 * @brief Calls the script's `onInit` function, if present and valid.
		 *
		 * Intended to be called once, after all entities in the scene have
		 * been loaded (see ScriptComponentInterface::initializeAllScripts).
		 */
		void callInitFunction() const
		{
			if (onInitSolFunction.valid())
				onInitSolFunction();
		}

		/**
		 * @brief Calls the script's `onCollision` function.
		 * @param otherEntity The entity involved in the collision.
		 */
		void triggerCollision(ecs::entity otherEntity)
		{
			if (onCollisionSolFunction.valid()) [[likely]]
			onCollisionSolFunction(otherEntity);
		}

		/**
		 * @brief Loads (or reloads) this collision script from a Lua file.
		 *
		 * Parses and executes @p scriptPath, expecting it to return a Lua
		 * table containing an `onCollision` function, and optionally an
		 * `onInit` function. On success, the resolved Lua functions
		 * replace this instance's bound functions; on failure, this
		 * instance's previously-bound functions (if any) are left
		 * untouched.
		 *
		 * @param scriptPath Path to the `.lua` file to load.
		 * @return true if the script was loaded and validated successfully,
		 *         false on a syntax error, runtime error, or if the
		 *         returned table is missing/malformed.
		 */
		bool loadScript(const std::filesystem::path& scriptPath)
		{
			sol::load_result loadResult = s_SolHandle.load_file(scriptPath);
			if (!loadResult.valid())
			{
				sol::error err = loadResult;
				clz::log::error(
					"SYNTAX error in sensor sript: " +
					scriptPath.string() + ": " + 
					err.what());
				return false;

			}
			sol::protected_function_result result = s_SolHandle.script_file(scriptPath);
			if (!result.valid())
			{
				sol::error err = result;
				clz::log::error("RUNTIME error in sensor sript: " +
						scriptPath.string() + " " + 
						err.what());
				return false;
			}

			sol::object returned = result;
			if (returned.get_type() != sol::type::table)
			{
				clz::log::error("Collision Script: " + scriptPath.string() + 
						" does not return a table");
				return false;
			}

			sol::table returnedTable = returned;
			sol::object collisionFunc = returnedTable["onCollision"];
			if (!collisionFunc.valid())
			{
				clz::log::error("Collision Script: " + scriptPath.string() + 
						" does not have 'onCollision' function");
				return false;
			}
			if (collisionFunc.get_type() != sol::type::function)
			{
				clz::log::error("Collision Script: " + scriptPath.string() + 
						" has 'onCollision' but its not a function wtf!!??");
				return false;
			}

			onCollisionSolFunction = collisionFunc;

			/// --- call the onInit function, if it exists --- ///
			sol::object initObj = returnedTable["onInit"];
			if (initObj.valid() && initObj.get_type() == sol::type::function)
			{
				onInitSolFunction = initObj;
			}
			else
			{
				clz::log::warn("Sensor script: " + 
						scriptPath.string() + 
						" has no 'onInit function'");
			}

			return true;
		}
	};


}
