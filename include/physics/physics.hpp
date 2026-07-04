#pragma once

#include <box3d/box3d.h>

namespace clz::physics
{
	inline b3WorldId worldId = b3_nullWorldId;
	constexpr auto gravity = (b3Vec3){0.0f, -10.0f, 0.0f};

	inline bool init()
	{
		b3WorldDef worldDef;
		worldDef.gravity = gravity;

		worldId = b3CreateWorld(&worldDef);

		return true;
	}
} // namespace clz::physics