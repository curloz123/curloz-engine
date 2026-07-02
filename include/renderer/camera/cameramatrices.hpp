#pragma once

#include "math/mat4x4.hpp"
#include <optional>

namespace clz::renderer::camera
{
	inline std::optional<math::mat4> view;
	inline std::optional<math::mat4> projection;

	math::mat4 getViewMatrix();
	math::mat4 getProjectionMatrix();
}