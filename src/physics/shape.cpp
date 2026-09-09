/**
@file shape.cpp
@author curl0z
@brief Box3D's shape implementation file
*/
#include "physics/shape.hpp"
#include "physics/body.hpp"
#include "physics/math.hpp"
#include "physics/physics.hpp"

namespace clz::physics
{
	/// @copydoc
	Shape::Shape(
		const ShapeDef& shapeDef, 
		const RigidBodyId rigidBodyId, 
		const RigidBodyShapeId shapeId)
	{
		createShape(shapeDef, rigidBodyId, shapeId);
	}

	/// @copydoc
	void Shape::createShape(
			const ShapeDef& shapeDef, 
			const RigidBodyId rigidBodyId, 
			const RigidBodyShapeId shapeId)
	{
		/// Set internal data
		m_externalId = shapeId;
		m_shapeType = shapeDef.shapeType;
		m_position = shapeDef.position;
		m_rotation = shapeDef.rotation;
		m_isSensor = shapeDef.isSensor;

		const b3Vec3 pos = toVec3(m_position);
		const b3Quat quat = toQuat(math::quatFromEuler(m_rotation));
		const b3Transform localTransform = {pos, quat};

		b3ShapeDef sDef = b3DefaultShapeDef();
		sDef.density = shapeDef.density;
		sDef.baseMaterial.friction = shapeDef.friction;
		sDef.baseMaterial.restitution = shapeDef.restitution;

		sDef.isSensor = m_isSensor;
		sDef.enableSensorEvents = shapeDef.enableSensorEvents;
		sDef.enableContactEvents = shapeDef.enableContactEvents;
		sDef.enableHitEvents = shapeDef.enableHitEvents;

		switch (m_shapeType)
		{
		case (ShapeType::BOX): {
			m_halfExtents = shapeDef.halfExtents;
			const b3BoxHull cuboid = b3MakeTransformedBoxHull(
				m_halfExtents.x,
				m_halfExtents.y,
				m_halfExtents.z,
				localTransform
			);
			m_shapeId =
				b3CreateHullShape(getBox3dBodyId(rigidBodyId), &sDef, &cuboid.base);
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "Unable to create shape");
			break;
		}
		case (ShapeType::SPHERE): {
			m_radius = shapeDef.radius;
			break;
		}
		case (ShapeType::CAPSULE): {
			m_radius = shapeDef.radius;
			m_height = shapeDef.height;
			break;
		}
		case (ShapeType::CYLINDER): {
			m_radius = shapeDef.radius;
			m_height = shapeDef.height;
			break;
		}
		}

		///< Apply mass
		b3Body_ApplyMassFromShapes(getBox3dBodyId(rigidBodyId));


		m_needsRecreation = false;
		m_shouldBeDestroyed = false;
	}

	/// @copydoc
	void Shape::destroyShape(const bool isRecreating)
	{
		b3DestroyShape(m_shapeId, true);
		m_shapeId = b3_nullShapeId;
		if (!isRecreating)
			m_shouldBeDestroyed = true;
	}

	/// @copydoc
	void Shape::recreateShape(const RigidBodyId rigidBodyId)
	{
		const ShapeDef shapeDef(
			this->getShapeType(),
			m_position,
			m_rotation,
			this->getDensity(),
			this->getFriction(),
			this->getRestitution(),
			m_halfExtents,
			m_radius,
			m_height,
			m_isSensor,
			this->areSensorEventsEnabled(),
			this->areContactEventsEnabled(),
			this->areHitEventsEnabled()
		);
		this->destroyShape(true);
		this->createShape(shapeDef, rigidBodyId, m_externalId);

		/// mark it un-outdated
		m_needsRecreation = false;
	}
} // namespace clz::physics
