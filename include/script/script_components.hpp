#pragma once

#include <sol/sol.hpp>
#include "entity/entitymanager.hpp"
#include "physics_scripts.hpp"
#include <unordered_map>
#include <vector>

namespace clz::script
{
	template <typename T>
	class ScriptComponentInterface
	{
	private:
		static inline std::vector<T> s_scriptLUT;
		static inline std::vector<std::string> s_scriptPaths;
		static inline std::unordered_map<std::string, std::uint32_t> s_pathToScriptMap;

		std::vector<uint32_t> m_scriptIds;
	public:
		void loadScript(const std::filesystem::path& scriptPath)
		{
			T script;
			if (script.loadScript(scriptPath))
			{
				std::uint32_t scriptId = s_scriptLUT.size();
				m_scriptIds.push_back(scriptId);
				s_scriptLUT.push_back(std::move(script));
				s_scriptPaths.emplace_back(scriptPath.string());
				s_pathToScriptMap[scriptPath.string()] = scriptId;

				clz::log::debug("loaded script: " + scriptPath.string());
			}
			else
			{
				clz::log::error("Unable to load script: " + scriptPath.string());
			}
		}

		template<typename... Args>
		void callFunctionInAllScripts(
			void (T::*fn)(Args...) const, 
			const Args... args) const
		{
			for (uint32_t Id : m_scriptIds)
			{
				(s_scriptLUT[Id].*fn)(args...);
			}
		}

		/// Must be called after all entities have been loaded
		static void initializeAllScripts()
		{
			for (const T& script : s_scriptLUT)
			{
				script.callInitFunction();
			}
		}

		void getScriptPaths(std::vector<std::string>& rScriptPaths) const
		{
			rScriptPaths.reserve(m_scriptIds.size());
			for (auto i : m_scriptIds)
			{
				rScriptPaths.emplace_back(s_scriptPaths[i]);
			}
		}
	};

	class SensorScriptComponent : public ScriptComponentInterface<SensorScript>
	{};
	class CollisionScriptComponent : public ScriptComponentInterface<CollisionScript>
	{};


}
