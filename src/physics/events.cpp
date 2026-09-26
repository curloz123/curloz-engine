/**
 * file sensor_events.cpp
 * @author curl0z
 * @brief manages all sensor events in physics system
 */

#include "physics/events.hpp"
#include "box3d/box3d.h"
#include "box3d/math_functions.h"
#include "box3d/types.h"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "entity/entitymanager.hpp"
#include "physics/math.hpp"
#include "physics/physics_types.hpp"
#include "physics/body.hpp"
#include "script/script_components.hpp"
#include "physics/shape.hpp"

namespace clz::physics
{
	/// @copydoc processSensorEvents
	void processSensorEvents()
	{
		///< Get all events from box3d
		b3SensorEvents sensorEvents = b3World_GetSensorEvents(p_world);

		///< Process all begin touch events
		for (int i = 0; i < sensorEvents.beginCount; ++i)
		{
			b3SensorBeginTouchEvent* beginTouch = sensorEvents.beginEvents + i;

			b3ShapeId sensorShape = beginTouch->sensorShapeId;
			b3BodyId sensorBody = b3Shape_GetBody(sensorShape);
			b3ShapeId visitorShape = beginTouch->visitorShapeId;
			b3BodyId visitorBody = b3Shape_GetBody(visitorShape);

			ecs::entity sensorEntity = getAttachedEntity(sensorBody);
			ecs::entity visitorEntity = getAttachedEntity(visitorBody);

			/// --- Let script do rest of work --- ///
			if (ecs::hasComponent<script::SensorScriptComponent>(sensorEntity))
			{
				const auto sensorScript = ecs::getComponent
								<script::SensorScriptComponent>(sensorEntity);
				sensorScript.callFunctionInAllScripts(
					&script::SensorScript::triggerOnSensorEnter, 
					visitorEntity);
			}
		}

		///< Process all end touch events
		for (int i = 0; i < sensorEvents.endCount; ++i)
		{
			b3SensorEndTouchEvent* endTouch = sensorEvents.endEvents + i;

			b3ShapeId sensorShape = endTouch->sensorShapeId;
			b3BodyId sensorBody = b3Shape_GetBody(sensorShape);
			b3ShapeId visitorShape = endTouch->visitorShapeId;
			b3BodyId visitorBody = b3Shape_GetBody(visitorShape);

			ecs::entity sensorEntity = getAttachedEntity(sensorBody);
			ecs::entity visitorEntity = getAttachedEntity(visitorBody);

			/// --- Let script do rest of work --- ///
			if (ecs::hasComponent<script::SensorScriptComponent>(sensorEntity))
			{
				const auto& sensorScriptComp = ecs::getComponent
								<script::SensorScriptComponent>(sensorEntity);
				sensorScriptComp.callFunctionInAllScripts(
					&script::SensorScript::triggerOnSensorExit, 
					visitorEntity);
			}
		}
	}

	/// @copydoc processCollisionEvents
	void processCollisionEvents()
	{
		b3ContactEvents contactEvents = b3World_GetContactEvents(p_world);

		/// OPTIMIZE THIS PART
		for (int i = 0; i < contactEvents.beginCount; ++i)
		{
			b3ContactBeginTouchEvent* beginEvent = contactEvents.beginEvents + i;

			b3ShapeId sensorShapeA = beginEvent->shapeIdA;
			b3BodyId sensorBodyA = b3Shape_GetBody(sensorShapeA);

			b3ShapeId sensorShapeB = beginEvent->shapeIdB;
			b3BodyId sensorBodyB = b3Shape_GetBody(sensorShapeB);
			
			ecs::entity entityA = getAttachedEntity(sensorBodyA);
			ecs::entity entityB = getAttachedEntity(sensorBodyB);
			
			math::vec3 posA;
			math::vec3 posB;
			if (!b3Shape_AreHitEventsEnabled(sensorShapeA) && 
				b3Shape_AreHitEventsEnabled(sensorShapeB))
			{
				posA = ecs::getComponent<ecs::TransformComponent>(entityA).position;
				posB = ecs::getComponent<ecs::TransformComponent>(entityB).position;
			}
			else
			{
				for (int j = 0; j < contactEvents.hitCount; ++j)
				{
					b3ContactHitEvent* hitEvent = contactEvents.hitEvents + j;
					posA = fromVec3((b3Vec3)hitEvent->point);
					posB = fromVec3((b3Vec3)hitEvent->point);
				}
			}

			if (ecs::hasComponent<script::CollisionScriptComponent>(entityA))
			{
				auto& collScriptComp = 
					ecs::getComponent<script::CollisionScriptComponent>(entityA);
				collScriptComp.callFunctionInAllScripts(
					&script::CollisionScript::triggerCollision,
					entityB,
					posA
				);
			}
			if (ecs::hasComponent<script::CollisionScriptComponent>(entityB))
			{
				auto& collScriptComp = 
					ecs::getComponent<script::CollisionScriptComponent>(entityB);
				collScriptComp.callFunctionInAllScripts(
					&script::CollisionScript::triggerCollision,
					entityA,
					posB
				);
			}

		}
	}
}
