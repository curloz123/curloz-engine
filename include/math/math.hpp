#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace clz::math
{
	// Vector types
	using vec2 = glm::vec2;
	using vec3 = glm::vec3;
	using vec4 = glm::vec4;

	// Matrix types
	using mat3 = glm::mat3;
	using mat4 = glm::mat4;

	// Quaternion
	using quat = glm::quat;

	// Common functions — just forwarding to GLM
	using glm::radians;
	using glm::degrees;
	using glm::normalize;
	using glm::cross;
	using glm::dot;
	using glm::length;
	using glm::translate;
	using glm::rotate;
	using glm::scale;
	using glm::perspective;
	using glm::lookAt;
	using glm::value_ptr;

} // namespace clz::math