#pragma once

#include "vec3.hpp"
#include "quat.hpp"
#include <cmath>
#include <algorithm>

namespace clz::math
{
	inline math::vec3 lerp(const vec3& begin, const vec3& end, const float alpha)
	{
		return begin + (end - begin) * alpha;
	}

	inline math::quat slerp(const math::quat& begin, const math::quat& end, const float alpha)
	{
		float d = dot(begin, end);
		float negate = 1.0f;
		if (d < 0.0f)
		{
			negate = -1.0f;
			d = -d;
		}
		const float cosine = std::clamp(d, -1.0f, 1.0f);

		if (cosine > 0.9995f)
		{
			return normalize(
				begin + ((end*negate) - begin) * alpha);
		}

		const float angle = std::acos(cosine);

		const quat q1 = (std::sin((1 - alpha) * angle)) / std::sin(angle) * begin;
		const quat q2 = (std::sin(alpha * angle)) / std::sin(angle) * (end * negate);
		return normalize(q1 + q2);
	}
}