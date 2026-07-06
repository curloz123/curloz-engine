/**
 * @file body.cpp
 * @author curl0z
 * @brief Box3D's body implementation file
 */
#include "physics/body.hpp"
#include "physics/physics_types.hpp"
#include "physics/math.hpp"

namespace clz::physics
{
	/// @brief Creates a new rigid body in the physics world from the given definition
	/// @param def Body definition data (type, mass, transform, damping, sleep, locks)
	/// @return Handle to the newly created body
	b3BodyId createBody(const BodyData& def)
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		bodyDef.type = static_cast<b3BodyType>(def.type);
		bodyDef.position = toVec3(def.position);
		bodyDef.rotation = toQuat(def.rotation);
		bodyDef.linearDamping = def.linearDamping;
		bodyDef.angularDamping = def.angularDamping;
		bodyDef.enableSleep = def.enableSleep;
		bodyDef.motionLocks = b3MotionLocks
		{
			.linearX = def.linearLocks[0],
			.linearY = def.linearLocks[1],
			.linearZ = def.linearLocks[2],
			.angularX = def.angularLocks[0],
			.angularY = def.angularLocks[1],
			.angularZ = def.angularLocks[2]
		};
		// Apply mass properties
		b3BodyId bodyId = b3CreateBody(p_world, &bodyDef);
		b3MassData myMassData;
		myMassData.mass = def.mass;
		myMassData.center = (b3Vec3){0.0f, 0.0f, 0.0f};
		myMassData.inertia = b3Mat3_identity;
		b3Body_SetMassData(bodyId, myMassData);
		return bodyId;
	}

	/// @brief Destroys a rigid body and invalidates its handle
	/// @param bodyId Handle reference to the body to destroy
	void destroyBody(b3BodyId& bodyId)
	{
		b3DestroyBody(bodyId);
		bodyId = b3_nullBodyId;
	}

	/// @brief Sets the mass of a body, preserving its current center of mass and rotational inertia
	/// @param bodyId Handle to the target body
	/// @param mass New mass value
	void setMass(const b3BodyId bodyId, const float mass)
	{
		b3MassData massData
		{
			.mass = mass,
			.center = b3Body_GetLocalCenterOfMass(bodyId),
			.inertia = b3Body_GetLocalRotationalInertia(bodyId)
		};
		b3Body_SetMassData(bodyId, massData);
	}

	/// @brief Sets the world-space position of a body, preserving its current rotation
	/// @param bodyId Handle to the target body
	/// @param position New world-space position
	void setBodyPosition(const b3BodyId bodyId, const math::vec3& position)
	{
		b3Body_SetTransform(bodyId, toVec3(position),
					b3Body_GetRotation(bodyId));
	}

	/// @brief Sets the world-space rotation of a body, preserving its current position
	/// @param bodyId Handle to the target body
	/// @param rotation New world-space rotation
	void setBodyRotation(const b3BodyId bodyId, const math::quat& rotation)
	{
		b3Body_SetTransform(bodyId, b3Body_GetPosition(bodyId),
				toQuat(rotation));
	}

	/// @brief Sets the linear damping coefficient of a body
	/// @param bodyId Handle to the target body
	/// @param linearDamping New linear damping value
	void setBodyLinearDamping(const b3BodyId bodyId, const float linearDamping)
	{
		b3Body_SetLinearDamping(bodyId, linearDamping);
	}

	/// @brief Sets the angular damping coefficient of a body
	/// @param bodyId Handle to the target body
	/// @param angularDamping New angular damping value
	void setBodyAngularDamping(const b3BodyId bodyId, const float angularDamping)
	{
		b3Body_SetAngularDamping(bodyId, angularDamping);
	}

	/// @brief Enables or disables sleeping for a body
	/// @param bodyId Handle to the target body
	/// @param enable True to allow the body to sleep, false to keep it always awake
	void enableSleep(const b3BodyId bodyId, const bool enable)
	{
		b3Body_EnableSleep(bodyId, enable);
	}

	/// @brief Sets which linear axes (X, Y, Z) are locked for a body
	/// @param bodyId Handle to the target body
	/// @param linearLocks Array of 3 bools indicating locked linear axes (X, Y, Z)
	void setLinearLock(const b3BodyId bodyId, const std::array<bool, 3>& linearLocks)
	{
		b3MotionLocks current = b3Body_GetMotionLocks(bodyId);
		current.linearX = linearLocks[0];
		current.linearY = linearLocks[1];
		current.linearZ = linearLocks[2];
		b3Body_SetMotionLocks(bodyId, current);
	}

	/// @brief Sets which angular axes (X, Y, Z) are locked for a body
	/// @param bodyId Handle to the target body
	/// @param angularLocks Array of 3 bools indicating locked angular axes (X, Y, Z)
	void setAngularLock(const b3BodyId bodyId, const std::array<bool, 3>& angularLocks)
	{
		b3MotionLocks current = b3Body_GetMotionLocks(bodyId);
		current.angularX = angularLocks[0];
		current.angularY = angularLocks[1];
		current.angularZ = angularLocks[2];
		b3Body_SetMotionLocks(bodyId, current);
	}
}