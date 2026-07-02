#pragma once

#include <immintrin.h>
#include <cmath>

namespace clz::math
{
	struct quat
	{
		union
		{
			__m128 xmm;
			struct
			{
				float w;
				float x;
				float y;
				float z;
			};
		};

		quat() : w(1.0f), x(0.0f), y(0.0f), z(0.0f)
		{

		}

		quat(const float w, const float x,
			const float y, const float z) :
				xmm(_mm_set_ps(z, y, x, w))
		{
		}

		explicit quat(const __m128 xmm) : xmm(xmm)
		{
		}

		quat operator*(const quat &q) const
		{
			return {w*q.w - x*q.x - y*q.y - x*q.z,
					w*q.x + x*q.w + y*q.z - z*q.y,
					w*q.y - x*q.z + y*q.w + z*q.x,
					w*q.z + x*q.y - y*q.x + z*q.w};
		}

	};



}