#include <sol/sol.hpp>
#include "entity/entitymanager.hpp"
#include "core/logs.hpp"
#include "native.hpp"

namespace clz::script
{
	struct SensorScript
	{
	private:
		sol::function onInitSolFunction;
		sol::function onEnterSolFunction;	
		sol::function onExitSolFunction;
	public:
		void callInitFunction() const
		{
			if (onInitSolFunction.valid())
				onInitSolFunction();
		}
		void triggerOnSensorEnter(const ecs::entity otherEntity) const
		{
			if (onEnterSolFunction.valid()) [[likely]]
				onEnterSolFunction(otherEntity);
		}
		void triggerOnSensorExit(const ecs::entity otherEntity) const
		{
			if (onExitSolFunction.valid()) [[likely]]
				onExitSolFunction(otherEntity);
		}

		bool loadScript(const std::filesystem::path& scriptPath)
		{
			sol::protected_function_result result = s_SolHandle.script_file(scriptPath);
			if (!result.valid())
			{
				sol::error err = result;
				clz::log::error("Failed to load sensor sript: " +
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

			if (!beginFunc.valid() && !endFunc.valid())
			{
				clz::log::error("Sensor Script: " + scriptPath.string() + 
						" is missing onSensorBegin or onSensorEnd"
						" functions");
				return false;
			}
			if (beginFunc.get_type() != sol::type::function &&
				    endFunc.get_type() != sol::type::function)
			{
				clz::log::error("Sensor Script: " + scriptPath.string() + 
						" has onSensorBegin or onSensorEnd"
						" that are not functions");
				return false;

			}

			onEnterSolFunction = beginFunc;
			onExitSolFunction = endFunc;

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
	struct CollisionScript
	{
	private:
		sol::function onInitSolFunction;
		sol::function onCollisionSolFunction;	
	public:
		void callInitFunction() const
		{
			if (onInitSolFunction.valid())
				onInitSolFunction();
		}
		void triggerCollision(ecs::entity otherEntity)
		{
			if (onCollisionSolFunction.valid()) [[likely]]
			onCollisionSolFunction(otherEntity);
		}

		bool loadScript(const std::filesystem::path& scriptPath)
		{
			sol::protected_function_result result = 
				s_SolHandle.script_file(scriptPath);
			if (!result.valid())
			{
				sol::error err = result;
				clz::log::error("Failed to load collision script: " + 
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
