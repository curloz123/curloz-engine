#pragma once

#include "script/script_components.hpp"

namespace clz::script
{
	inline void flagScriptSystemEntitiesLoaded()
	{
		SensorScriptComponent::initializeAllScripts();
		CollisionScriptComponent::initializeAllScripts();
	}
}
