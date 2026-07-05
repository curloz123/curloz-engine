#pragma once

#include "core/logs.hpp"
#include <box3d/box3d.h>

namespace clz::physics
{
	inline b3WorldId worldId = b3_nullWorldId;
	constexpr auto gravity = (b3Vec3){0.0f, -10.0f, 0.0f};

	inline bool init()
	{
		b3WorldDef worldDef = b3DefaultWorldDef();
		worldDef.gravity = gravity;

		worldId = b3CreateWorld(&worldDef);

		b3BodyDef groundBodyDef = b3DefaultBodyDef();
		groundBodyDef.position = (b3Vec3){0.0f, -10.0f, 0.0f};
		b3BodyId bodyId = b3CreateBody(worldId, &groundBodyDef);

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.baseMaterial.friction = 0.3f;

		b3Sphere sphere;
		sphere.center = (b3Vec3){2.0f, 3.0f, 0.0f};
		sphere.radius = 0.5f;
		b3CreateSphereShape(bodyId, &shapeDef, &sphere);

		b3BoxHull box = b3MakeBoxHull(0.5f, 1.0f, 0.5f);
		b3CreateHullShape(bodyId, &shapeDef, &box.base);

		clz::log::debug("shapeCount = " +
			std::to_string(b3Body_GetShapeCount(bodyId)));


		b3Transform transform;

		return true;
	}
} // namespace clz::physics