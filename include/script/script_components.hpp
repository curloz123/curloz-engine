/**
 * @file script_components.hpp
 * @author curl0z
 * @brief Defines all script system components
 */

#pragma once

#include <sol/sol.hpp>
#include "physics_scripts.hpp"
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clz::script
{
	/**
	 * @brief Generic ECS component wrapper pooling scripts of type T.
	 *
	 * Provides shared, path-deduplicated storage for a given script type
	 * `T` (e.g. SensorScript, CollisionScript). All entities that load the
	 * same script file share a single underlying `T` instance, looked up
	 * by path; each component instance only stores the IDs of the scripts
	 * it references.
	 *
	 * @tparam T The script type this interface manages. Must provide:
	 *           - `bool loadScript(const std::filesystem::path&)`
	 *           - `void callInitFunction() const`
	 *           - any additional `const` member functions intended to be
	 *             dispatched via callFunctionInAllScripts().
	 *
	 * @note `s_scriptLUT`, `s_scriptPaths`, and `s_pathToScriptMap` are
	 *       `static`, and therefore shared across every entity's
	 *       component of this specialization of `T` — not per-instance.
	 */
	template <typename T>
	class ScriptComponentInterface
	{
	private:
		/// Shared pool of all loaded scripts of type T, indexed by script ID.
		static inline std::vector<T> s_scriptLUT;
		/// Shared pool of script file paths, parallel to #s_scriptLUT.
		static inline std::vector<std::string> s_scriptPaths;
		/// Maps a script's file path to its ID (index into #s_scriptLUT).
		static inline std::unordered_map<std::string, std::uint32_t> s_pathToScriptMap;

		/// IDs (into #s_scriptLUT) of the scripts referenced by this component instance.
		std::vector<uint32_t> m_scriptIds;
	public:
		/**
		 * @brief Loads a script from disk, or reloads it if already loaded.
		 *
		 * If @p scriptPath has already been loaded by any component
		 * instance (of this specialization of T), the existing pooled
		 * script is re-loaded in place (hot reload) and this component
		 * instance is made to reference it, without duplicating storage.
		 * If the reload fails, the previously-bound (working) script is
		 * left untouched and continues executing.
		 *
		 * If @p scriptPath has not been loaded before, a new `T` instance
		 * is constructed, loaded, and appended to the shared pool. If the
		 * initial load fails, the script is still registered as a
		 * component (so it can later be fixed and reloaded via this same
		 * path), but its behavior will be inert until it loads successfully.
		 *
		 * @param scriptPath Path to the `.lua` script file to load.
		 */
		void loadScript(const std::filesystem::path& scriptPath)
		{
			if (auto it = s_pathToScriptMap.find(scriptPath.string()); 
				it != s_pathToScriptMap.end())
			{
				if (s_scriptLUT[it->second].loadScript(scriptPath))
				{
					clz::log::debug("reloaded script: " + scriptPath.string());
					/// Initialize script
					s_scriptLUT[it->second].callInitFunction();
				}
				else
					clz::log::error("Unable to reload script: " + scriptPath.string() +
							" Previous version of script would be executed"
							", Until this version is fixed");


				bool exists = std::ranges::any_of(m_scriptIds, [&it](const uint32_t scriptId){
							return scriptId == it->second;
						});
				if (!exists)
					m_scriptIds.push_back(it->second);
				

				return;
			}

			T script;
			if (script.loadScript(scriptPath))
				clz::log::debug("loaded script: " + scriptPath.string());
			else
				clz::log::error("Unable to load script: " + scriptPath.string() +
						" It will be added as a component"
						", but won't be executed");

			std::uint32_t scriptId = s_scriptLUT.size();
			m_scriptIds.push_back(scriptId);
			s_scriptLUT.push_back(std::move(script));
			s_scriptPaths.emplace_back(scriptPath.string());
			s_pathToScriptMap[scriptPath.string()] = scriptId;
			clz::log::debug("vec size: " + std::to_string(m_scriptIds.size()));
		}

		/**
		 * @brief Calls a given const member function on every script this
		 *        component references.
		 *
		 * Dispatches @p fn, a pointer to a const member function of `T`,
		 * to every script in #m_scriptIds, forwarding @p args to each call.
		 * Used to trigger script callbacks (e.g. `triggerOnSensorEnter`,
		 * `triggerCollision`) without going through `std::function`
		 * type erasure.
		 *
		 * @tparam Args Parameter types of the member function being called.
		 * @param fn   Pointer to the const member function of T to invoke.
		 * @param args Arguments forwarded to each call of @p fn.
		 */
		template<typename... Args>
		void callFunctionInAllScripts(
			void (T::*fn)(Args...) const, 
			std::type_identity_t<Args>... args) const
		{
			for (uint32_t Id : m_scriptIds)
			{
				(s_scriptLUT[Id].*fn)(args...);
			}
		}

		/**
		 * @brief Calls `callInitFunction()` on every pooled script of type T.
		 *
		 * @note Must be called after all entities have been loaded, so that
		 *       any entity lookups performed by a script's `onInit`
		 *       (e.g. resolving another entity by name) succeed.
		 */
		static void initializeAllScripts()
		{
			for (const T& script : s_scriptLUT)
			{
				script.callInitFunction();
			}
		}

		/**
		 * @brief Removes a script reference from this component instance.
		 *
		 * Removes the ID associated with @p scriptPath from this
		 * component's #m_scriptIds. Does not remove the underlying script
		 * from the shared pool (#s_scriptLUT), since other component
		 * instances may still reference the same script by ID.
		 *
		 * @param scriptPath Path of the script to remove from this component.
		 */
		void removeScript(const std::filesystem::path& scriptPath)
		{
			auto it = s_pathToScriptMap.find(scriptPath.string());
			if (it == s_pathToScriptMap.end())
			{
				clz::log::warn("No such script found: " + scriptPath.string());
				return;
			}

			std::erase_if(
				m_scriptIds, 
				[&it](const uint32_t scriptId){
					return scriptId == it->second;
				}
			);
			clz::log::debug("Removed script: " + scriptPath.string());
		}

		/**
		 * @brief Retrieves the file paths of every script this component references.
		 * @param[out] rScriptPaths Vector to append the resolved script paths to.
		 */
		void getScriptPaths(std::vector<std::string>& rScriptPaths) const
		{
			rScriptPaths.reserve(m_scriptIds.size());
			for (auto i : m_scriptIds)
			{
				rScriptPaths.emplace_back(s_scriptPaths[i]);
			}
		}
	};

	/// ECS component wrapping SensorScript instances (physics sensor/trigger callbacks).
	class SensorScriptComponent : public ScriptComponentInterface<SensorScript>
	{};

	/// ECS component wrapping CollisionScript instances (physics collision callbacks).
	class CollisionScriptComponent : public ScriptComponentInterface<CollisionScript>
	{};

}
