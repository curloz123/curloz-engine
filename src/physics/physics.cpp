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
#include "math/interpolate.hpp"
#include "physics/body.hpp"
#include "physics/physics_types.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"

namespace clz::physics
{
	/// @brief Initializes the physics engine
	bool init()
	{
		b3WorldDef worldDef = b3DefaultWorldDef();

		// No multithreading for now
		p_gravity = (b3Vec3){0.0f, -9.8f, 0.0f};
		worldDef.gravity = p_gravity;
		worldDef.enableSleep = config::getBool("physics", "enablesleep", false);
		p_world = b3CreateWorld(&worldDef);
		if (B3_IS_NULL(p_world))
		{
			clz::log::error("Failed to create physics world");
			return false;
		}

		p_timeStep = config::getFloat("physics", "timestep", 0.0167f);
		p_subStepCount = config::getInt("physics", "substepcount", 4);
		p_accumulator = 0.0f;

		clz::log::info("Created physics world");
		return true;
	}

	/// @brief Update's the physics engine
	/// @note in editor mode, the physics engine reads EditorTransformComponent of the entity,
	/// and writes back data to internal data structure. Cuz we don't want physics mingling while
	/// we're editing do we??
	/// Also we use fixed time step with that remainder accumulator method
	/// Uses slerp and lerp for smooth transition of transform
	void update()
	{
#ifdef CLZ_ENABLE_EDITOR
		if (state::g_engineState != state::EngineState::Game)
		{
			for (auto& entities = ecs::getEntitiesWithComponent<ecs::BodyComponent>(); auto& entity : entities)
			{
				auto& body = ecs::getComponent<ecs::BodyComponent>(entity);
				const auto& transformComponent = ecs::getComponent<ecs::EditorTransformComponent>(entity);
				static size_t c = 0;
				setBodyPosition(body.bodyId, transformComponent.position);
				setBodyRotation(body.bodyId, transformComponent.rotation);
				body.newRotation = transformComponent.rotation;
				body.prevRotation = body.newRotation;
				body.newPosition = transformComponent.position;
				body.prevPosition = body.newPosition;

				b3Body_SetLinearVelocity(body.bodyId, (b3Vec3){0.0f, 0.0f, 0.0f});
				b3Body_SetAngularVelocity(body.bodyId, (b3Vec3){0.0f, 0.0f, 0.0f});
			}
			return;
		}
#endif

		p_accumulator += time::getDeltaTime();
		const auto& entities = ecs::getEntitiesWithComponent<ecs::BodyComponent>();
		while (p_accumulator >= p_timeStep)
		{
			b3World_Step(p_world, p_timeStep, p_subStepCount);
			p_accumulator -= p_timeStep;

			for (const auto& entity : entities)
			{
				auto& body = ecs::getComponent<ecs::BodyComponent>(entity);
				body.prevPosition = body.newPosition;
				body.prevRotation = body.newRotation;
				body.newPosition = getBodyPosition(body.bodyId);
				body.newRotation = getBodyRotation(body.bodyId);
			}
		}
		const float alpha = p_accumulator / p_timeStep;

		for (const auto& entity : entities)
		{
			auto& transformComponent = ecs::getComponent<ecs::TransformComponent>(entity);
			const auto& body = ecs::getComponent<ecs::BodyComponent>(entity);

			transformComponent.position = math::lerp(body.prevPosition, body.newPosition, alpha);
			transformComponent.rotation = math::slerp(body.prevRotation, body.newRotation, alpha);
		}
	}

	/// @brief Shuts down the physics engine
	void shutdown()
	{
		b3DestroyWorld(p_world);
		clz::log::info("Destroyed physics world");
	}
} // namespace clz::physics
