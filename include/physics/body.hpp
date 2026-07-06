/**
 * @file body.hpp
 * @author curl0z
 * @brief Box3D's body main header interface
 */

#pragma once
#include <box3d/box3d.h>
#include "math/vec3.hpp"
#include "math/quat.hpp"
#include <array>

namespace clz::physics
{
	/// @brief For external use
	using BodyId = b3BodyId;

	/// @brief BodyType
	enum class BodyType
	{
		StaticBody = b3_staticBody,
		KinematicBody = b3_kinematicBody,
		DynamicBody = b3_dynamicBody
	};

	/// @brief Body definition data
	struct BodyData
	{
		BodyType type = BodyType::DynamicBody;
		float mass = 1.0f;
		math::vec3 position = math::vec3(0.0f, 0.0f, 0.0f);
		math::quat rotation = math::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float linearDamping = 0.0f;
		float angularDamping = 0.1f;
		bool enableSleep = false;
		std::array<bool, 3> linearLocks = {false, false, false};
		std::array<bool, 3> angularLocks = {false, false, false};
	};
}

namespace clz::physics
{
	/// @brief Creates a new rigid body in the physics world from the given definition
	/// @param def Body definition data (type, mass, transform, damping, sleep, locks)
	/// @return Handle to the newly created body
	b3BodyId createBody(const BodyData& def);

	/// @brief Destroys a rigid body and invalidates its handle
	/// @param bodyId Handle to the body to destroy
	void destroyBody(b3BodyId& bodyId);

	/// @brief Sets the mass of a body, preserving its current center of mass and rotational inertia
	/// @param bodyId Handle to the target body
	/// @param mass New mass value
	void setMass(b3BodyId bodyId, float mass);

	/// @brief Sets the world-space position of a body, preserving its current rotation
	/// @param bodyId Handle to the target body
	/// @param position New world-space position
	void setBodyPosition(b3BodyId bodyId, const math::vec3& position);

	/// @brief Sets the world-space rotation of a body, preserving its current position
	/// @param bodyId Handle to the target body
	/// @param rotation New world-space rotation
	void setBodyRotation(b3BodyId bodyId, const math::quat& rotation);

	/// @brief Sets the linear damping coefficient of a body
	/// @param bodyId Handle to the target body
	/// @param linearDamping New linear damping value
	void setBodyLinearDamping(b3BodyId bodyId, float linearDamping);

	/// @brief Sets the angular damping coefficient of a body
	/// @param bodyId Handle to the target body
	/// @param angularDamping New angular damping value
	void setBodyAngularDamping(b3BodyId bodyId, float angularDamping);

	/// @brief Enables or disables sleeping for a body
	/// @param bodyId Handle to the target body
	/// @param enable True to allow the body to sleep, false to keep it always awake
	void enableSleep(b3BodyId bodyId, bool enable);

	/// @brief Sets which linear axes (X, Y, Z) are locked for a body
	/// @param bodyId Handle to the target body
	/// @param linearLocks Array of 3 bools indicating locked linear axes (X, Y, Z)
	void setLinearLock(b3BodyId bodyId, const std::array<bool, 3>& linearLocks);

	/// @brief Sets which angular axes (X, Y, Z) are locked for a body
	/// @param bodyId Handle to the target body
	/// @param angularLocks Array of 3 bools indicating locked angular axes (X, Y, Z)
	void setAngularLock(b3BodyId bodyId, const std::array<bool, 3>& angularLocks);
}