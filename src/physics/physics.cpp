#include "physics/physics.hpp"
#include "physics/physics_types.hpp"
#include "core/logs.hpp"
#include "config/config.hpp"

namespace clz::physics
{
	bool init()
	{
		p_err = ERROR::None;

		b3WorldDef worldDef = b3DefaultWorldDef();
		worldDef.enableSleep = config::getBool("physics", "enablesleep", false);
		// No multithreading for now

		p_gravity = (b3Vec3){0.0f, -10.0f, 0.0f};
		worldDef.gravity = p_gravity;
		p_world = b3CreateWorld(&worldDef);
		if (B3_IS_NULL(p_world))
		{
			clz::log::error("Failed to create physics world");
			p_err = ERROR::WorldCreation;
			return false;
		}

		p_timeStep = config::getFloat("physics", "timestep", 0.0167f);
		p_subStepCount = config::getInt("physics", "substepcount", 4);

		clz::log::info("Created physics world");
		return true;
	}

	void update()
	{
		b3World_Step(p_world, p_timeStep, p_subStepCount);
	}


	void shutdown()
	{
		switch (p_err)
		{
		case None:
			b3DestroyWorld(p_world);
			clz::log::info("Destroyed physics world");

		case ERROR::WorldCreation:
			clz::log::info("Not destroying world in physics");

		case ERROR::UnInitialized:
			clz::log::info("Physics was uninitialized, not destroying anything");
		}
	}
}
