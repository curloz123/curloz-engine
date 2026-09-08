#include "physics/sensor_events.hpp"
#include "box3d/box3d.h"
#include "box3d/types.h"
#include "entity/entitymanager.hpp"
#include "physics/physics_types.hpp"
#include "physics/body.hpp"
#include <string>

namespace clz::physics
{
	void processSensorEvents()
	{
		///< Get all events from box3d
		b3SensorEvents sensorEvents = b3World_GetSensorEvents(p_world);


		///< Process all begin touch events
		for (int i = 0; i < sensorEvents.beginCount; ++i)
		{
			b3SensorBeginTouchEvent* beginTouch = sensorEvents.beginEvents + i;

			b3ShapeId shapeA = beginTouch->sensorShapeId;
			b3BodyId bodyA = b3Shape_GetBody(shapeA);
			b3ShapeId shapeB = beginTouch->visitorShapeId;
			b3BodyId bodyB = b3Shape_GetBody(shapeB);

			ecs::entity entityA = getAttachedEntity(bodyA);
			ecs::entity entityB = getAttachedEntity(bodyB);

			clz::log::debug("Sensor begin event: entityA: " + 
					ecs::getEntityName(entityA) + 
					" entityB: " + 
					ecs::getEntityName(entityB));
		}

		///< Process all end touch events
		for (int i = 0; i < sensorEvents.endCount; ++i)
		{
			b3SensorEndTouchEvent* endTouch = sensorEvents.endEvents + i;

			b3ShapeId shapeA = endTouch->sensorShapeId;
			b3BodyId bodyA = b3Shape_GetBody(shapeA);
			b3ShapeId shapeB = endTouch->visitorShapeId;
			b3BodyId bodyB = b3Shape_GetBody(shapeB);

			ecs::entity entityA = getAttachedEntity(bodyA);
			ecs::entity entityB = getAttachedEntity(bodyB);

			clz::log::debug("Sensor end event: entityA: " + 
					ecs::getEntityName(entityA) + 
					" entityB: " + 
					ecs::getEntityName(entityB));
		}
	}
}
