/**
@file shape.cpp
@author curl0z
@brief Box3D's shape implementation file
*/
#include "physics/shape.hpp"
#include "physics/body.hpp"
#include "physics/math.hpp"

namespace clz::physics
{
	/// @copydoc
	Shape::Shape(const ShapeDef& shapeDef, const RigidBodyId rigidBodyId)
	{
		createShape(shapeDef, rigidBodyId);
	}

	/// @copydoc
	void Shape::createShape(const ShapeDef& shapeDef, const RigidBodyId rigidBodyId)
	{
		m_shapeType = shapeDef.shapeType;
		m_position = shapeDef.position;
		m_rotation = shapeDef.rotation;
		const b3Vec3 pos = toVec3(m_position);
		const b3Quat quat = toQuat(math::quatFromEuler(m_rotation));
		const b3Transform localTransform = {pos, quat};

		b3ShapeDef sDef = b3DefaultShapeDef();
		sDef.density = shapeDef.density;
		sDef.baseMaterial.friction = shapeDef.friction;
		sDef.baseMaterial.restitution = shapeDef.restitution;

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
		b3Body_ApplyMassFromShapes(getBox3dBodyId(rigidBodyId));
		needsRecreation = false;
		shouldBeDestroyed = false;
	}

	/// @copydoc
	void Shape::destroyShape(const bool isRecreating)
	{
		b3DestroyShape(m_shapeId, true);
		m_shapeId = b3_nullShapeId;
		if (!isRecreating)
			shouldBeDestroyed = true;
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
			m_height
		);
		this->destroyShape(true);
		this->createShape(shapeDef, rigidBodyId);
		needsRecreation = false;
	}
} // namespace clz::physics