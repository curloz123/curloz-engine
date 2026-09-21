/**
 * file sensor_events.cpp
 * @author curl0z
 * @brief manages all sensor events in physics system
 */

#include "physics/sensor_events.hpp"
#include "box3d/box3d.h"
#include "box3d/types.h"
#include "entity/componentmanager.hpp"
#include "entity/entitymanager.hpp"
#include "physics/physics_types.hpp"
#include "physics/body.hpp"
#include "script/script_components.hpp"

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

			clz::log::debug("Sensor begin from engine");

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
			clz::log::debug("Sensor end from engine");

			/// --- Let script do rest of work --- ///
			if (ecs::hasComponent<script::SensorScriptComponent>(sensorEntity))
			{
				const auto sensorScript = ecs::getComponent
								<script::SensorScriptComponent>(sensorEntity);
				sensorScript.callFunctionInAllScripts(
					&script::SensorScript::triggerOnSensorExit, 
					visitorEntity);
			}
		}
	}
}
