/**
 * @file body.hpp
 * @author curl0z
 * @brief Box3D's body main header interface
 */

#pragma once
#include "math/quat.hpp"
#include "math/vec3.hpp"
#include "shape.hpp"
#include <array>
#include <box3d/box3d.h>
#include <vector>

namespace clz::physics
{
	inline uint32_t numRigidBodies = 0;
	inline constexpr
		uint32_t NULL_RIGID_BODY = UINT32_MAX;

	/// @brief BodyType
	enum class BodyType
	{
		StaticBody = b3_staticBody,
		KinematicBody = b3_kinematicBody,
		DynamicBody = b3_dynamicBody
	};

	/// --- All these data's are indexed via RigidBodyId
	inline std::vector<b3BodyId> Bodies;
	inline std::vector<std::vector<Shape>> Shapes;

	/// @brief Body definition data
	struct BodyData
	{
		BodyType type = BodyType::DynamicBody;
		bool enableSleep = true;
		math::vec3 position = math::vec3(0.0f, 0.0f, 0.0f);
		math::quat rotation = math::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float linearDamping = 0.0f;
		float angularDamping = 0.1f;
		std::array<bool, 3> linearLocks = {false, false, false};
		std::array<bool, 3> angularLocks = {false, false, false};

		std::vector<ShapeDef> ShapeDefs = {};
	};
} // namespace clz::physics

namespace clz::physics
{
	/// @brief Creates a new rigid body in the physics world from the given definition
	/// @param def Body definition data (type, mass, transform, damping, sleep, locks)
	/// @return Handle to the newly created body
	RigidBodyId createBody(BodyData& def);

	/// @brief Disables a rigid body and invalidates its handle
	/// Done because mid-editing, deleting is too costly,
	/// also helps in undo-redo
	/// @param rigidBodyId Handle to the body to disable
	void disableBody(RigidBodyId rigidBodyId);

	/// @brief re-enables a body, similar reason to disabling
	/// @param rigidBodyId Handle to the body to destroy
	void enableBody(RigidBodyId rigidBodyId);

	BodyData getBodyData(RigidBodyId rigidBodyId);
	b3BodyId getBox3dBodyId(RigidBodyId rigidBodyId);

	void attachShapeToBody(
		RigidBodyId rigidBodyId,
		const ShapeDef& shapeDef);
	std::vector<Shape>& getBodyShapes(RigidBodyId rigidBodyId);

	void refreshAttachedShapes(RigidBodyId rigidBodyId);

	/// @brief Returns body type (kinematic, static, dynamic)
	/// @param rigidBodyId ID of body to retrieve data
	/// @return Body's type
	BodyType getBodyType(RigidBodyId rigidBodyId);

	/// @brief Sets body type (kinematic, static, dynamic)
	/// @param rigidBodyId ID of body to retrieve data
	/// @param type Which type to set the body to
	void setBodyType(RigidBodyId rigidBodyId, BodyType type);

	/**
	 * @brief Retrieve mass of body
	 * @note mass can only be retrieved via physics engine.
	 * And mass properties are automatically computed via the engine.
	 * As of now, there's no way to set them, as it should be.
	 * @param rigidBodyId ID of body to retrieve data
	 * @return Mass of the body
	 */
	float getBodyMass(RigidBodyId rigidBodyId);

	/// @brief Sets the world-space position of a body, preserving its current rotation
	/// @param rigidBodyId Handle to the target body
	/// @param position New world-space position
	void setBodyPosition(RigidBodyId rigidBodyId, const math::vec3& position);

	/// @brief returns calculated position of rigid body
	/// Usually called after updating physics world
	/// @param rigidBodyId Id of body
	/// @return math::vec3 Position of body
	math::vec3 getBodyPosition(RigidBodyId rigidBodyId);

	/// @brief Sets the world-space rotation of a body, preserving its current position
	/// @param rigidBodyId Handle to the target body
	/// @param rotation New world-space rotation
	void setBodyRotation(RigidBodyId rigidBodyId, const math::quat& rotation);
	/**
	 * @brief Returns calculated rotation of body
	 * Usually called after updating the physics world.
	 * @param rigidBodyId Handle to the target body
	 * @return Rotation of body
	 */
	math::quat getBodyRotation(RigidBodyId rigidBodyId);

	/// @brief Sets the linear damping coefficient of a body
	/// @param rigidBodyId Handle to the target body
	/// @param linearDamping New linear damping value
	void setBodyLinearDamping(RigidBodyId rigidBodyId, float linearDamping);
	/// @brief retrieves linear damping of a body
	/// @param rigidBodyId Id of body to retrieve data
	/// @return linear damping of body
	float getBodyLinearDamping(RigidBodyId rigidBodyId);

	/// @brief Sets the angular damping coefficient of a body
	/// @param rigidBodyId Handle to the target body
	/// @param angularDamping New angular damping value
	void setBodyAngularDamping(RigidBodyId rigidBodyId, float angularDamping);
	/// @brief Retrieves angular damping of body
	/// @param rigidBodyId ID of body to retrieve data
	/// @return angular damping of body
	float getBodyAngularDamping(const RigidBodyId rigidBodyId);

	/// @brief Enables or disables sleeping for a body
	/// @param rigidBodyId Handle to the target body
	/// @param enable True to allow the body to sleep, false to keep it always awake
	void enableSleep(RigidBodyId rigidBodyId, bool enable);
	/// @brief Retrieves whether body is sleeping
	/// @param rigidBodyId ID of body to retrieve data
	/// @return Is sleeping enabled??
	bool isSleepEnabled(RigidBodyId rigidBodyId);

	/// @brief Sets which linear axes (X, Y, Z) are locked for a body
	/// @param rigidBodyId Handle to the target body
	/// @param linearLocks Array of 3 bools indicating locked linear axes (X, Y, Z)
	void setBodyLinearLocks(
		RigidBodyId rigidBodyId,
		const std::array<bool, 3>& linearLocks);

	/// @brief retrieve linear locks of a body
	/// @param rigidBodyId Id of body to retrieve data
	/// @return Size - 3 array containing lock info of xyz axis
	std::array<bool, 3> getBodyLinearLocks(RigidBodyId rigidBodyId);

	/// @brief Sets which angular axes (X, Y, Z) are locked for a body
	/// @param rigidBodyId Handle to the target body
	/// @param angularLocks Array of 3 bools indicating locked angular axes (X, Y, Z)
	void setBodyAngularLocks(
		RigidBodyId rigidBodyId,
		const std::array<bool, 3>& angularLocks);

	/// @brief retrieves angular locks info of a body
	/// @param rigidBodyId Id of body to retrieve data
	/// @return Array of 3 bools indicating locked angular axes
	std::array<bool, 3> getBodyAngularLocks(const RigidBodyId rigidBodyId);

	void setBodyVelocity(RigidBodyId rigidBodyId, const math::vec3& velocity);
	math::vec3 getBodyVelocity(RigidBodyId rigidBodyId);
	void setBodyAngularVelocity(RigidBodyId rigidBodyId, const math::vec3& velocity);
	math::vec3 getBodyAngularVelocity(RigidBodyId rigidBodyId);
} // namespace clz::physics