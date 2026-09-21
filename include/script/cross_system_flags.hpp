/**
 * @file cross_system_flags.hpp
 * @author curl0z
 * @brief cross system flags header for script system
 */

#pragma once

#include "script/script_components.hpp"

namespace clz::script
{
	/**
	 * @brief Flags the script system that all scene entities have finished loading.
	 *
	 * Calls `initializeAllScripts()` on both SensorScriptComponent and
	 * CollisionScriptComponent, running each pooled script's `onInit`
	 * Lua callback. Must be called once, after scene/entity loading has
	 * fully completed, so that scripts performing entity lookups in
	 * `onInit` (e.g. resolving another entity by name) can find them.
	 */
	inline void flagScriptSystemEntitiesLoaded()
	{
		SensorScriptComponent::initializeAllScripts();
		CollisionScriptComponent::initializeAllScripts();
	}
}
