/**
 * @file physics.cpp
 * @author curl0z
 * @brief Main physics implementation file
 */

#include "physics/physics.hpp"
#include "config/config.hpp"
#include "core/enginestate.hpp"
#include "core/logs.hpp"
#include "core/time.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "math/interpolate.hpp"
#include "physics/body.hpp"
#include "physics/physics_types.hpp"
#include "physics/physicscomponent.hpp"
#include "physics/math.hpp"
#include "physics/events.hpp"

namespace clz::physics
{
	/// @brief Initializes the physics engine
	bool init()
	{
		b3WorldDef worldDef = b3DefaultWorldDef();

		// No multithreading for now
		p_gravity = b3Vec3{0.0f, -9.8f, 0.0f};
		worldDef.gravity = p_gravity;
		worldDef.enableSleep = config::getValue<bool>("physics", "enablesleep", false);
		p_world = b3CreateWorld(&worldDef);
		if (B3_IS_NULL(p_world))
		{
			clz::log::error("Failed to create physics world");
			return false;
		}

		p_timeStep = config::getValue<float>("physics", "timestep", 0.0167f);
		p_subStepCount = config::getValue<int>("physics", "substepcount", 4);
		p_accumulator = 0.0f;

		clz::log::info("Created physics world");
		return true;
	}

	/// @brief Update's the physics engine
	/// @note in editor mode, we don't iterate physics engine at all
	/// @note Also we use fixed time step with
	/// that remainder accumulator method Uses slerp and lerp for smooth transition of
	/// transform
	void update()
	{
		if (state::g_engineState != state::EngineState::Game)
		{
			return;
		}

		p_accumulator += time::getDeltaTime();
		const auto& entities = ecs::getEntitiesWithComponent<RigidBodyComponent>();
		while (p_accumulator >= p_timeStep)
		{
			b3World_Step(p_world, p_timeStep, p_subStepCount);
			p_accumulator -= p_timeStep;

			for (const auto& entity : entities)
			{
				auto& body = ecs::getComponent<RigidBodyComponent>(entity);

				body.prevPosition = body.newPosition;
				body.prevRotation = body.newRotation;
				body.newPosition = getBodyPosition(body.rigidBodyId);
				body.newRotation = getBodyRotation(body.rigidBodyId);

			}

			///< Process sensor events
			processSensorEvents();
			///< Process collision events
			processCollisionEvents();
		}
		const float alpha = p_accumulator / p_timeStep;

		///< update entity's transform
		for (const auto& entity : entities)
		{
			auto& transformComponent =
				ecs::getComponent<ecs::TransformComponent>(entity);
			const auto& body = ecs::getComponent<RigidBodyComponent>(entity);

			transformComponent.position =
				math::lerp(body.prevPosition, body.newPosition, alpha);
			transformComponent.rotation =
				math::slerp(body.prevRotation, body.newRotation, alpha);
		}

	}

	/// @brief Shuts down the physics engine
	void shutdown()
	{
		b3DestroyWorld(p_world);
		clz::log::info("Destroyed physics world");
	}
} // namespace clz::physics
