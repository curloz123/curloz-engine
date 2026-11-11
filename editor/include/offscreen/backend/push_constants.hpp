/**
 * @file push_constants.hpp
 * @author curl0z
 * @brief All pipelines push constants must be defined here
 */

#pragma once

#include "math/mat4x4.hpp"
#include <cstdint>

namespace clz::editor::backend
{
	///< @brief body editor pipeline's push constant
	struct BodyEditorPC
	{
		/// pass mvp combined
		math::mat4 mvp;

		/// material index of model
		uint32_t materialIndex;
	};
}
