/**
 * @file listener_manager.hpp
 * @author curl0z
 * @brief Provides audio listener related functions
 */

#pragma once

#include "native.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/camera/cameradata.hpp"
#include "renderer/renderer.hpp"
#include "math/vec3.hpp"
#include "core/enginestate.hpp"
#include <array>
#ifdef CLZ_ENABLE_EDITOR
#endif

namespace clz::audio
{
	/// @brief Updates listener data
	/// @note Internally retrieves camera's data. 
	/// Must be called only after entities have loaded
	inline void updateListenerData()
	{
	}
}
