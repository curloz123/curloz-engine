#pragma once

#include "core/logs.hpp"
#include "core/enginestate.hpp"
#include "physicscomponent.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"

namespace clz::physics
{
	static void resetRigidBodies();
}
namespace clz::physics
{
	inline void flagPhysicsStateChange(
		const clz::state::EngineState oldState,
		const clz::state::EngineState newState
	)
	{
		clz::log::info("state change hinted to physics system");
		if ((newState == clz::state::EngineState::Editor && oldState == clz::state::EngineState:: Game) ||
			(newState == clz::state::EngineState::Game && oldState == clz::state::EngineState::Editor))
		{
			resetRigidBodies();
		}
	}

	static void resetRigidBodies()
	{
		for (auto& entities = ecs::getEntitiesWithComponent<RigidBodyComponent>();
			auto& entity : entities)
		{
			auto& body = ecs::getComponent<RigidBodyComponent>(entity);
			const auto& transformComponent =
				ecs::getComponent<ecs::TransformComponent>(entity);

			setBodyPosition(body.rigidBodyId, transformComponent.position);
			body.newPosition = transformComponent.position;
			body.prevPosition = body.newPosition;

			const auto bodyRotation = transformComponent.rotation;
			setBodyRotation(body.rigidBodyId, bodyRotation);

			body.newRotation = bodyRotation;
			body.prevRotation = bodyRotation;

			setBodyVelocity(body.rigidBodyId, {0.0f, 0.0f, 0.0f});
			setBodyAngularVelocity(body.rigidBodyId, {0.0f, 0.0f, 0.0f});
		}

	}
}
