/**
 * file sensor_events.cpp
 * @author curl0z
 * @brief manages all sensor events in physics system
 */

#include "physics/events.hpp"
#include "box3d/box3d.h"
#include "box3d/math_functions.h"
#include "math/vec3.hpp"
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

	/// @copydoc processHitEvents
	void processHitEvents()
	{
		b3ContactEvents contactEvents = b3World_GetContactEvents(p_world);

		/// OPTIMIZE THIS PART
		static 	std::vector<
				std::pair<
					std::pair<ecs::entity, ecs::entity>,
					std::pair<math::vec3, float>
				>
			> entt_contact_detes;
		entt_contact_detes.reserve(contactEvents.hitCount);

		for (int i = 0; i < contactEvents.hitCount; ++i)
		{
			b3ContactHitEvent* hitEvent = contactEvents.hitEvents + i;

			b3ShapeId sensorShapeA = hitEvent->shapeIdA;
			b3ShapeId sensorShapeB = hitEvent->shapeIdB;
			b3BodyId sensorBodyA = b3Shape_GetBody(sensorShapeA);
			b3BodyId sensorBodyB = b3Shape_GetBody(sensorShapeB);
			ecs::entity entityA = getAttachedEntity(sensorBodyA);
			ecs::entity entityB = getAttachedEntity(sensorBodyB);
			
			entt_contact_detes.emplace_back(
				std::make_pair(
					std::make_pair(
						entityA, 
						entityB
					),
					std::make_pair(
						fromVec3(hitEvent->point), 
						hitEvent->approachSpeed
					)
				)
			);
			
		}

		for (const auto& contact_dete : entt_contact_detes)
		{
			const auto& entt_pair = contact_dete.first;
			const auto& contact_detes = contact_dete.second;

			if (ecs::hasComponent<script::CollisionScriptComponent>(entt_pair.first))
			{
				const auto& collScriptComp = 
					ecs::getComponent<script::CollisionScriptComponent>(entt_pair.first);
				collScriptComp.callFunctionInAllScripts(
					&script::CollisionScript::triggerCollision,
					entt_pair.second,
					contact_detes.first,
					contact_detes.second
				);
			}
			if (ecs::hasComponent<script::CollisionScriptComponent>(entt_pair.second))
			{
				const auto& collScriptComp = 
					ecs::getComponent<script::CollisionScriptComponent>(entt_pair.second);
				collScriptComp.callFunctionInAllScripts(
					&script::CollisionScript::triggerCollision,
					entt_pair.first,
					contact_detes.first,
					contact_detes.second
				);
			}
		}

		entt_contact_detes.clear();
	}
}
