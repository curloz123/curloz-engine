/**
 * @file body.cpp
 * @author curl0z
 * @brief Box3D's body implementation file
 */
#include "physics/body.hpp"
#include "math/quateulerconv.hpp"
#include "physics/physics_types.hpp"
#include "physics/math.hpp"
#include "physics/shape.hpp"
#include <box3d/box3d.h>
#include "core/logs.hpp"
#include "core/assert.hpp"

namespace clz::physics
{
	/**
	 * @brief Creates a new rigid body in the physics world from the given definition
	 * @param def Body definition data (type, mass, transform, damping, sleep, locks)
	 * @return Handle to the newly created body
	 */
	b3BodyId createBody(BodyData& def)
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

		b3BodyId bodyId = b3CreateBody(p_world, &bodyDef);

		return bodyId;
	}

	/// @brief Destroys a rigid body and invalidates its handle
	/// @param bodyId Handle reference to the body to destroy
	void destroyBody(b3BodyId& bodyId)
	{
		b3DestroyBody(bodyId);
		bodyId = b3_nullBodyId;
	}

	/**
	 * @brief Changes body type the body.
	 * Accepted types - Static, Kinematic, Dynamic
	 * @param bodyId ID of body
	 * @param type Change type to???
	 */
	void setBodyType(const b3BodyId bodyId, const BodyType type)
	{
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
	/**
	 * @brief Retrieves which type of body,
	 * the body is (bad english T_T)
	 * @param bodyId ID of body
	 * @return Type of body
	 */
	BodyType getBodyType(const b3BodyId bodyId)
	{
		switch (b3Body_GetType(bodyId))
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
	/**
	 * @brief Retrieve mass of body
	 * @note mass can only be retrieved via physics engine.
	 * And mass properties are automaticall computed via the engine.
	 * As of now, there's no way to set them, as it should be.
	 * @param bodyId ID of body to retrieve data
	 * @return Mass of the body
	 */
	float getBodyMass(const b3BodyId bodyId)
	{
		return b3Body_GetMass(bodyId);
	}

	/**
	 * @brief Sets the world-space position of a body, preserving its current rotation
	 * @param bodyId Handle to the target body
	 * @param position New world-space position
	 */
	void setBodyPosition(const b3BodyId bodyId, const math::vec3& position)
	{
		b3Body_SetTransform(bodyId, toVec3(position),
					b3Body_GetRotation(bodyId));
	}

	/**
	 * @brief returns calculated position of rigid body
	 * Usually called after updating physics world
	 * @param bodyId Id of body
	 * @return math::vec3 Position of body
	 */
	math::vec3 getBodyPosition(const b3BodyId bodyId)
	{
		return fromVec3(b3Body_GetPosition(bodyId));
	}

	/**
	 * @brief Sets the world-space rotation of a body, preserving its current position
	 * @param bodyId Handle to the target body
	 * @param rotation New world-space rotation
	 */
	void setBodyRotation(const b3BodyId bodyId, const math::quat& rotation)
	{
		b3Body_SetTransform(bodyId, b3Body_GetPosition(bodyId),
				toQuat(rotation));
	}

	/**
	 * @brief Returns calculated rotation of body
	 * Usually called after updating the physics world.
	 * @param bodyId Handle to the target body
	 * @return Rotation of body
	 */
	math::quat getBodyRotation(const b3BodyId bodyId)
	{
		return fromQuat(b3Body_GetRotation(bodyId));
	}

	/// @brief Sets the linear damping coefficient of a body
	/// @param bodyId Handle to the target body
	/// @param linearDamping New linear damping value
	void setBodyLinearDamping(const b3BodyId bodyId, const float linearDamping)
	{
		b3Body_SetLinearDamping(bodyId, linearDamping);
	}
	/**
	* @brief retrieves linear damping of a body
	* @param bodyId Id of body to retrieve data
	* @return linear damping od body
	*/
	float getBodyLinearDamping(const b3BodyId bodyId)
	{
		return b3Body_GetLinearDamping(bodyId);
	}

	/// @brief Sets the angular damping coefficient of a body
	/// @param bodyId Handle to the target body
	/// @param angularDamping New angular damping value
	void setBodyAngularDamping(const b3BodyId bodyId, const float angularDamping)
	{
		b3Body_SetAngularDamping(bodyId, angularDamping);
	}
	/**
	 * @brief Retrieves angular damping of body
	 * @param bodyId ID of body to retrieve data
	 * @return angular damping of body
	 */
	float getBodyAngularDamping(const b3BodyId bodyId)
	{
		return b3Body_GetAngularDamping(bodyId);
	}

	/// @brief Enables or disables sleeping for a body
	/// @param bodyId Handle to the target body
	/// @param enable True to allow the body to sleep, false to keep it always awake
	void enableSleep(const b3BodyId bodyId, const bool enable)
	{
		b3Body_EnableSleep(bodyId, enable);
	}
	/**
	 * @brief Retrieves whether body is sleeping
	 * @param bodyId ID of body to retrieve data
	 * @return Is sleeping enabled??
	 */
	bool isSleepEnabled(const b3BodyId bodyId)
	{
		return b3Body_IsSleepEnabled(bodyId);
	}

	/// @brief Sets which linear axes (X, Y, Z) are locked for a body
	/// @param bodyId Handle to the target body
	/// @param linearLocks Array of 3 bools indicating locked linear axes (X, Y, Z)
	void setBodyLinearLocks(const b3BodyId bodyId, const std::array<bool, 3>& linearLocks)
	{
		b3MotionLocks current = b3Body_GetMotionLocks(bodyId);
		current.linearX = linearLocks[0];
		current.linearY = linearLocks[1];
		current.linearZ = linearLocks[2];
		b3Body_SetMotionLocks(bodyId, current);
	}
	/**
	 * @brief retrieve linear locks of a body
	 * @param bodyId Id of body to retrieve data
	 * @return Size - 3 array containing lock info of xyz axis
	 */
	std::array<bool, 3> getBodyLinearLocks(const b3BodyId bodyId)
	{
		const b3MotionLocks current = b3Body_GetMotionLocks(bodyId);
		return {current.linearX, current.linearY, current.linearZ};
	}

	/// @brief Sets which angular axes (X, Y, Z) are locked for a body
	/// @param bodyId Handle to the target body
	/// @param angularLocks Array of 3 bool's indicating locked angular axes (X, Y, Z)
	void setBodyAngularLocks(const b3BodyId bodyId, const std::array<bool, 3>& angularLocks)
	{
		b3MotionLocks current = b3Body_GetMotionLocks(bodyId);
		current.angularX = angularLocks[0];
		current.angularY = angularLocks[1];
		current.angularZ = angularLocks[2];
		b3Body_SetMotionLocks(bodyId, current);
	}
	/**
	 * @brief retrieves angular locks info of a body
	 * @param bodyId Id of body to retrieve data
	 * @return Array of 3 bool's indicating locked angular axes
	 */
	std::array<bool, 3> getBodyAngularLocks(const b3BodyId bodyId)
	{
		const b3MotionLocks current = b3Body_GetMotionLocks(bodyId);
		return {current.angularX, current.angularY, current.angularZ};
	}
}