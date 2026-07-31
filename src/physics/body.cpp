/**
 * @file body.cpp
 * @author curl0z
 * @brief Box3D's body implementation file
 */
#include "physics/body.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "physics/math.hpp"
#include "physics/physics_types.hpp"
#include "physics/shape.hpp"
#include <box3d/box3d.h>

namespace clz::physics
{
	/// @copydoc
	RigidBodyId createBody(BodyData& def)
	{
		const auto rigidBodyId = numRigidBodies;
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = static_cast<b3BodyType>(def.type);
		bodyDef.position = toVec3(def.position);
		bodyDef.rotation = toQuat(def.rotation);
		bodyDef.linearDamping = def.linearDamping;
		bodyDef.angularDamping = def.angularDamping;
		bodyDef.enableSleep = def.enableSleep;
		bodyDef.motionLocks = b3MotionLocks{
			.linearX = def.linearLocks[0],
			.linearY = def.linearLocks[1],
			.linearZ = def.linearLocks[2],
			.angularX = def.angularLocks[0],
			.angularY = def.angularLocks[1],
			.angularZ = def.angularLocks[2]
		};

		const b3BodyId bodyId = b3CreateBody(p_world, &bodyDef);
		CLZ_ASSERT(B3_IS_NON_NULL(bodyId), "Unable to create body");

		Bodies.emplace_back(bodyId);
		++numRigidBodies;

		std::vector<Shape> shapes;
		for (const auto& shapeDef : def.ShapeDefs)
		{
			Shape shape(shapeDef, rigidBodyId);
			shapes.emplace_back(shape);
		}
		// that's why, always push back after attaching
		Shapes.emplace_back(std::move(shapes));
		return rigidBodyId;
	}

	/// @copydoc
	void attachShapeToBody(const RigidBodyId rigidBodyId,
			       const ShapeDef& shapeDef)
	{
		Shape shape(shapeDef, rigidBodyId);
		Shapes[rigidBodyId].emplace_back(shape);
	}

	/// @copydoc
	void disableBody(const RigidBodyId rigidBodyId)
	{
		CLZ_ASSERT(rigidBodyId < numRigidBodies,
			   "invalid id passed "
			   "while requesting disabling");
		b3Body_Disable(Bodies[rigidBodyId]);
	}

	/// @copydoc
	void enableBody(const RigidBodyId rigidBodyId)
	{
		CLZ_ASSERT(rigidBodyId < numRigidBodies,
			   "invalid id passed "
			   "while requesting enabling");
		b3Body_Enable(Bodies[rigidBodyId]);
	}

	/// @copydoc
	BodyData getBodyData(const RigidBodyId rigidBodyId)
	{
		return BodyData{
			.type = getBodyType(rigidBodyId),
			.enableSleep = isSleepEnabled(rigidBodyId),
			.position = getBodyPosition(rigidBodyId),
			.rotation = getBodyRotation(rigidBodyId),
			.linearDamping = getBodyLinearDamping(rigidBodyId),
			.angularDamping = getBodyAngularDamping(rigidBodyId),
			.linearLocks = getBodyLinearLocks(rigidBodyId),
			.angularLocks = getBodyAngularLocks(rigidBodyId),
		};
	}

	/// @copydoc
	b3BodyId getBox3dBodyId(const RigidBodyId rigidBodyId)
	{
		CLZ_ASSERT(rigidBodyId < numRigidBodies,
			   "invalid id enquired "
			   "while requesting internal body handle");
		return Bodies[rigidBodyId];
	}

	/// @copydoc
	std::vector<Shape>& getBodyShapes(const RigidBodyId rigidBodyId)
	{
		CLZ_ASSERT(rigidBodyId < numRigidBodies,
			   "invalid id enquired "
			   "while requesting body shapes");
		return Shapes[rigidBodyId];
	}

	/// @copydoc
	void refreshAttachedShapes(const RigidBodyId rigidBodyId)
	{
		auto& shapes = Shapes[rigidBodyId];
		std::erase_if(shapes, [](const Shape& shape) {
			return shape.isItTimeSon();
		});

		for (size_t i = 0; i < shapes.size(); ++i)
		{
			CLZ_ASSERT(B3_ID_EQUALS(Bodies[rigidBodyId],
						shapes[i].getAttachedBodyId()),
				   "Trying to recreate shapes of a body,"
				   "but passed shapes of another.");

			if (shapes[i].isOutdated())
			{
				shapes[i].recreateShape(rigidBodyId);
			}
		}
	}

	/// @copydoc
	void logBodyData(const RigidBodyId rigidBodyId)
	{
		clz::log::info("mass: " + std::to_string(getBodyMass(rigidBodyId)));
		clz::log::info("mass: " + std::to_string(getBodyMass(rigidBodyId)));
		clz::log::info("mass: " + std::to_string(getBodyMass(rigidBodyId)));
		clz::log::info("mass: " + std::to_string(getBodyMass(rigidBodyId)));
		clz::log::info("mass: " + std::to_string(getBodyMass(rigidBodyId)));
		clz::log::info("mass: " + std::to_string(getBodyMass(rigidBodyId)));
	}

	/// @copydoc
	void setBodyType(const RigidBodyId rigidBodyId, const BodyType type)
	{
		const auto bodyId = Bodies[rigidBodyId];
		switch (type)
		{
		case BodyType::StaticBody:
			b3Body_SetType(bodyId, b3_staticBody);
			break;
		case BodyType::KinematicBody:
			b3Body_SetType(bodyId, b3_kinematicBody);
			break;
		case BodyType::DynamicBody:
			b3Body_SetType(bodyId, b3_dynamicBody);
			break;
		default:
			CLZ_ASSERT(false, "invalid body type passed");
		}
	}

	/// @copydoc
	BodyType getBodyType(const RigidBodyId rigidBodyId)
	{
		switch (b3Body_GetType(Bodies[rigidBodyId]))
		{
		case b3_staticBody:
			return BodyType::StaticBody;
		case b3_kinematicBody:
			return BodyType::KinematicBody;
		case b3_dynamicBody:
			return BodyType::DynamicBody;
		default:
			CLZ_ASSERT(false, "invalid body type passed");
		}
	}

	/// @copydoc
	float getBodyMass(const RigidBodyId rigidBodyId)
	{
		return b3Body_GetMass(Bodies[rigidBodyId]);
	}

	/// @copydoc
	void setBodyPosition(const RigidBodyId rigidBodyId,
			     const math::vec3& position)
	{
		const auto& bodyId = Bodies[rigidBodyId];
		b3Body_SetTransform(bodyId,
				    toVec3(position),
				    toQuat(getBodyRotation(rigidBodyId)));
	}

	/// @copydoc
	math::vec3 getBodyPosition(const RigidBodyId rigidBodyId)
	{
		return fromVec3(b3Body_GetPosition(Bodies[rigidBodyId]));
	}

	/// @copydoc
	void setBodyRotation(const RigidBodyId rigidBodyId,
			     const math::quat& rotation)
	{
		const auto& bodyId = Bodies[rigidBodyId];
		b3Body_SetTransform(bodyId,
				    toVec3(getBodyPosition(rigidBodyId)),
				    toQuat(rotation));
	}

	/// @copydoc
	math::quat getBodyRotation(const RigidBodyId rigidBodyId)
	{
		return fromQuat(b3Body_GetRotation(Bodies[rigidBodyId]));
	}

	/// @copydoc
	void setBodyLinearDamping(const RigidBodyId rigidBodyId,
				  const float linearDamping)
	{
		b3Body_SetLinearDamping(Bodies[rigidBodyId], linearDamping);
	}

	/// @copydoc
	float getBodyLinearDamping(const RigidBodyId rigidBodyId)
	{
		return b3Body_GetLinearDamping(Bodies[rigidBodyId]);
	}

	/// @copydoc
	void setBodyAngularDamping(const RigidBodyId rigidBodyId,
				   const float angularDamping)
	{
		b3Body_SetAngularDamping(Bodies[rigidBodyId], angularDamping);
	}

	/// @copydoc
	float getBodyAngularDamping(const RigidBodyId rigidBodyId)
	{
		return b3Body_GetAngularDamping(Bodies[rigidBodyId]);
	}

	/// @copydoc
	void enableSleep(const RigidBodyId rigidBodyId, const bool enable)
	{
		b3Body_EnableSleep(Bodies[rigidBodyId], enable);
	}

	/// @copydoc
	bool isSleepEnabled(const RigidBodyId rigidBodyId)
	{
		return b3Body_IsSleepEnabled(Bodies[rigidBodyId]);
	}

	/// @copydoc
	void setBodyLinearLocks(const RigidBodyId rigidBodyId,
				const std::array<bool, 3>& linearLocks)
	{
		const auto& bodyId = Bodies[rigidBodyId];
		b3MotionLocks current = b3Body_GetMotionLocks(bodyId);
		current.linearX = linearLocks[0];
		current.linearY = linearLocks[1];
		current.linearZ = linearLocks[2];
		b3Body_SetMotionLocks(bodyId, current);
	}

	/// @copydoc
	std::array<bool, 3> getBodyLinearLocks(const RigidBodyId rigidBodyId)
	{
		const b3MotionLocks current =
			b3Body_GetMotionLocks(Bodies[rigidBodyId]);
		return {current.linearX, current.linearY, current.linearZ};
	}

	/// @copydoc
	void setBodyAngularLocks(const RigidBodyId rigidBodyId,
				 const std::array<bool, 3>& angularLocks)
	{
		const auto& bodyId = Bodies[rigidBodyId];
		b3MotionLocks current = b3Body_GetMotionLocks(bodyId);
		current.angularX = angularLocks[0];
		current.angularY = angularLocks[1];
		current.angularZ = angularLocks[2];
		b3Body_SetMotionLocks(bodyId, current);
	}

	/// @copydoc
	std::array<bool, 3> getBodyAngularLocks(const RigidBodyId rigidBodyId)
	{
		const b3MotionLocks current =
			b3Body_GetMotionLocks(Bodies[rigidBodyId]);
		return {current.angularX, current.angularY, current.angularZ};
	}

	/// @copydoc
	void setBodyVelocity(const RigidBodyId rigidBodyId,
			     const math::vec3& velocity)
	{
		b3Body_SetLinearVelocity(Bodies[rigidBodyId], toVec3(velocity));
	}

	/// @copydoc
	math::vec3 getBodyVelocity(const RigidBodyId rigidBodyId)
	{
		const auto vel = b3Body_GetLinearVelocity(Bodies[rigidBodyId]);
		return {vel.x, vel.y, vel.z};
	}

	/// @copydoc
	void setBodyAngularVelocity(const RigidBodyId rigidBodyId,
				    const math::vec3& velocity)
	{
		b3Body_SetAngularVelocity(Bodies[rigidBodyId],
					  toVec3(velocity));
	}

	/// @copydoc
	math::vec3 getBodyAngularVelocity(const RigidBodyId rigidBodyId)
	{
		const auto velocity =
			b3Body_GetAngularVelocity(Bodies[rigidBodyId]);
		return {velocity.x, velocity.y, velocity.z};
	}
} // namespace clz::physics