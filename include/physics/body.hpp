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
#include "shape.hpp"

namespace clz::physics
{
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
		math::vec3 position = math::vec3(0.0f, 0.0f, 0.0f);
		math::quat rotation = math::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float linearDamping = 0.0f;
		float angularDamping = 0.1f;
		bool enableSleep = true;
		std::array<bool, 3> linearLocks = {false, false, false};
		std::array<bool, 3> angularLocks = {false, false, false};

		std::vector<BoxShape> boxShapes;
	};
}

namespace clz::physics
{
	/// @brief Creates a new rigid body in the physics world from the given definition
	/// @param def Body definition data (type, mass, transform, damping, sleep, locks)
	/// @return Handle to the newly created body
	b3BodyId createBody(BodyData& def);

	/// @brief Destroys a rigid body and invalidates its handle
	/// @param bodyId Handle to the body to destroy
	void destroyBody(b3BodyId& bodyId);

	BodyType getBodyType(b3BodyId bodyId);

	void setBodyType(b3BodyId bodyId, BodyType type);

	/**
	 * @brief Retrieve mass of body
	 * @note mass can only be retrieved via physics engine.
	 * And mass properties are automaticall computed via the engine.
	 * As of now, there's no way to set them, as it should be.
	 * @param bodyId ID of body to retrieve data
	 * @return Mass of the body
	 */
	float getBodyMass(const b3BodyId bodyId);

	/// @brief Sets the world-space position of a body, preserving its current rotation
	/// @param bodyId Handle to the target body
	/// @param position New world-space position
	void setBodyPosition(b3BodyId bodyId, const math::vec3& position);
	/**
	 * @brief returns calculated position of rigid body
	 * Usually called after updating physics world
	 * @param bodyId Id of body
	 * @return math::vec3 Position of body
	 */
	math::vec3 getBodyPosition(const b3BodyId bodyId);

	/// @brief Sets the world-space rotation of a body, preserving its current position
	/// @param bodyId Handle to the target body
	/// @param rotation New world-space rotation
	void setBodyRotation(b3BodyId bodyId, const math::quat& rotation);
	/**
	 * @brief Returns calculated rotation of body
	 * Usually called after updating the physics world.
	 * @param bodyId Handle to the target body
	 * @return Rotation of body
	 */
	math::quat getBodyRotation(const b3BodyId bodyId);

	/// @brief Sets the linear damping coefficient of a body
	/// @param bodyId Handle to the target body
	/// @param linearDamping New linear damping value
	void setBodyLinearDamping(b3BodyId bodyId, float linearDamping);
	/**
	 * @brief retrieves linear damping of a body
	 * @param bodyId Id of body to retrieve data
	 * @return linear damping of body
	 */
	float getBodyLinearDamping(const b3BodyId bodyId);

	/// @brief Sets the angular damping coefficient of a body
	/// @param bodyId Handle to the target body
	/// @param angularDamping New angular damping value
	void setBodyAngularDamping(b3BodyId bodyId, float angularDamping);
	/**
	 * @brief Retrieves angular damping of body
	 * @param bodyId ID of body to retrieve data
	 * @return angular damping of body
	 */
	float getBodyAngularDamping(const b3BodyId bodyId);

	/// @brief Enables or disables sleeping for a body
	/// @param bodyId Handle to the target body
	/// @param enable True to allow the body to sleep, false to keep it always awake
	void enableSleep(b3BodyId bodyId, bool enable);
	/**
	 * @brief Retrieves whether body is sleeping
	 * @param bodyId ID of body to retrieve data
	 * @return Is sleeping enabled??
	 */
	bool isSleepEnabled(const b3BodyId bodyId);

	/// @brief Sets which linear axes (X, Y, Z) are locked for a body
	/// @param bodyId Handle to the target body
	/// @param linearLocks Array of 3 bools indicating locked linear axes (X, Y, Z)
	void setBodyLinearLocks(b3BodyId bodyId, const std::array<bool, 3>& linearLocks);
	/**
	 * @brief retrieve linear locks of a body
	 * @param bodyId Id of body to retrieve data
	 * @return Size - 3 array containing lock info of xyz axis
	 */
	std::array<bool, 3> getBodyLinearLocks(const b3BodyId bodyId);

	/// @brief Sets which angular axes (X, Y, Z) are locked for a body
	/// @param bodyId Handle to the target body
	/// @param angularLocks Array of 3 bools indicating locked angular axes (X, Y, Z)
	void setBodyAngularLocks(b3BodyId bodyId, const std::array<bool, 3>& angularLocks);
	/**
	 * @brief retrieves angular locks info of a body
	 * @param bodyId Id of body to retrieve data
	 * @return Array of 3 bools indicating locked angular axes
	 */
	std::array<bool, 3> getBodyAngularLocks(const b3BodyId bodyId);
}