#pragma once

#include "mat4x4.hpp"
#include "quat.hpp"
#include "vec3.hpp"
#include <cmath>

namespace clz::math
{
	inline mat4 getModelMatrix(const quat& r, const vec3& t, const vec3& s)
	{
		const mat4 rotation = {
			_mm_set_ps(0.0f, 2.0f * (r.x * r.z - r.y * r.w), 2.0f * (r.x * r.y + r.z * r.w),
				1.0f - 2.0f * (r.y * r.y + r.z * r.z)),
			_mm_set_ps(0.0f, 2.0f * (r.y * r.z + r.x * r.w), 1.0f - 2.0f * (r.x * r.x + r.z * r.z),
				2.0f * (r.x * r.y - r.z * r.w)),
			_mm_set_ps(0.0f, 1.0f - 2.0f * (r.x * r.x + r.y * r.y), 2.0f * (r.y * r.z - r.x * r.w),
				2.0f * (r.x * r.z + r.y * r.w)),
			_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f)
		};

		const mat4 translation = {_mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f), _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f),
			_mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f), _mm_set_ps(1.0f, t.z, t.y, t.x)};

		const auto scale = mat4(s);

		return scale * rotation * translation;
	}

	inline mat4 makeViewMatrix(const vec3& eye, const vec3& target, const vec3& worldUp)
	{
		const vec3 forward = normalize(subtract(target, eye));
		const vec3 right = normalize(cross(forward, worldUp));
		const vec3 up = cross(right, forward);

		mat4 view;
		view.r0 = _mm_set_ps(0.0f, -forward.x, up.x, right.x);
		view.r1 = _mm_set_ps(0.0f, -forward.y, up.y, right.y);
		view.r2 = _mm_set_ps(0.0f, -forward.z, up.z, right.z);
		view.r3 = _mm_set_ps(1.0f, dot(forward, eye), dot(up, eye) * -1, dot(right, eye) * -1);

		return view;
	}

	inline mat4 makePerspectiveMatrix(const float far, const float near, const float aspectRatio, const float fov)
	{
		const float sx = 1.0f / (aspectRatio * std::tan(fov * 0.5f));
		const float sy = -1.0f / std::tan(fov * 0.5f);

		const float a = far / (near - far);
		const float b = (far * near) / (near - far);

		return {_mm_set_ps(0.0f, 0.0f, 0.0f, sx), _mm_set_ps(0.0f, 0.0f, sy, 0.0f),
			_mm_set_ps(-1.0f, a, 0.0f, 0.0f), _mm_set_ps(0.0f, b, 0.0f, 0.0f)};
	}
} // namespace clz::math