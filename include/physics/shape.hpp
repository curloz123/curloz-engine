/**
 * @file shape.hpp
 * @author curl0z
 * @brief Physics shape header file
 * Provides BoxShape Data-Structure which holds
 * All the data of any shape attached to a body
 */
#pragma once

#include "core/assert.hpp"
#include "math/quateulerconv.hpp"
#include "math/vec3.hpp"
#include "physics.hpp"

namespace clz::physics
{
	enum class ShapeType
	{
		BOX,
		SPHERE,
		CAPSULE,
		CYLINDER
	};

	struct ShapeDef
	{
		ShapeType shapeType;

		math::vec3 position;
		math::vec3 rotation;

		float density;
		float friction;
		float restitution;

		math::vec3 halfExtents;
		float radius;
		float height;

		ShapeDef(){}
		explicit ShapeDef(
			const ShapeType shapeType,
			const math::vec3 position = math::vec3(0.0f, 0.0f, 0.0f),
			const math::vec3 rotation = math::vec3(0.0f, 0.0f, 0.0f),
			const float density = 0.0f,
			const float friction = 0.7f,
			const float restitution = 0.1f,
			const math::vec3 halfExtents = math::vec3(0.5f),
			const float radius = 0.5f,
			const float height = 1.0f)
		{
			this->shapeType = shapeType;
			this->position = position;
			this->rotation = rotation;
			this->density = density;
			this->friction = friction;
			this->restitution = restitution;
			this->halfExtents = halfExtents;
			this->radius = radius;
			this->height = height;
		}
	};
	class Shape
	{
	private:
		b3ShapeId m_shapeId;
		ShapeType m_shapeType;
		math::vec3 m_position;
		math::vec3 m_rotation;

		math::vec3 m_halfExtents;
		float m_radius;
		float m_height;

		bool needsRecreation = false;
		bool shouldBeDestroyed = false;
	public:
		Shape() {}
		Shape(const ShapeDef& shapeDef, RigidBodyId rigidBodyId);

		void createShape(const ShapeDef& shapeDef, RigidBodyId rigidBodyId);
		void destroyShape(bool isRecreating = false);
		void recreateShape(RigidBodyId rigidBodyId);

		ShapeDef getShapeData() const
		{
			return ShapeDef(
				this->getShapeType(),
				this->getPosition(),
				this->getRotation(),
				this->getDensity(),
				this->getFriction(),
				this->getRestitution(),
				this->m_halfExtents,
				this->m_radius,
				this->m_height);
		}

		bool isOutdated() const
		{
			return needsRecreation;
		}
		bool isItTimeSon() const
		{
			return shouldBeDestroyed;
		}

	    	ShapeType getShapeType() const
		{
			return m_shapeType;
		}
		b3BodyId getAttachedBodyId() const
		{
			return b3Shape_GetBody(m_shapeId);
		}
		b3ShapeId getShapeId() const
		{
			CLZ_ASSERT(
				B3_IS_NON_NULL(m_shapeId),
				"attempt to call shape getter on a null shape");

			return m_shapeId;
		}

	    	float getFriction() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters,"
				"unless it is attached to any body");
			return b3Shape_GetFriction(m_shapeId);
		}
	    	float getRestitution() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters,"
				"unless it is attached to any body");
			return b3Shape_GetRestitution(m_shapeId);
		}
	    	float getDensity() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters,"
				"unless it is attached to any body");
			return b3Shape_GetDensity(m_shapeId);
		}
	    	math::vec3 getPosition() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters,"
				"unless it is attached to any body");

			return m_position;
		}
	    	math::vec3 getRotation() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters,"
				"unless it is attached to any body");

			return m_rotation;
		}

	    	void setFriction(const float friction)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape setters,"
				"unless it is attached to any body");

			b3Shape_SetFriction(m_shapeId, friction);
		}
	    	void setRestitution(const float restitution)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape setters,"
				"unless it is attached to any body");

			b3Shape_SetRestitution(m_shapeId, restitution);
		}
	    	void setDensity(const float density)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape setters,"
				"unless it is attached to any body");
			b3Shape_SetDensity(
				m_shapeId,
				density,
				true);
		}
	    	void setPosition(const math::vec3 pos)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape setters,"
				"unless it is attached to any body");
			m_position = pos;
			needsRecreation = true;
		}
	    	void setRotation(const math::vec3 rot)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape setters,"
				"unless it is attached to any body");
			m_rotation = rot;
			needsRecreation = true;
		}

	    	math::vec3 getBoxHalfExtents() const
	    	{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters,"
				"unless it is attached to any body");
	    	    	return m_halfExtents;
	    	}

	    	float getSphereRadius() const
	    	{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape setters,"
				"unless it is attached to any body");
	    	    	return m_radius;
	    	}
		float getCapsuleRadius() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters, "
				"unless it is attached to any body");
			return m_radius;
		}

		float getCapsuleHeight() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters, "
				"unless it is attached to any body");
			return m_height;
		}

		float getCylinderRadius() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters, "
				"unless it is attached to any body");
			return m_radius;
		}

		float getCylinderHeight() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape getters, "
				"unless it is attached to any body");
			return m_height;
		}

		void setBoxHalfExtents(const math::vec3& extents)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape setters, "
				"unless it is attached to any body");
			m_halfExtents = extents;
			needsRecreation = true;
		}

		void setSphereRadius(const float r)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
				"You are not allowed to use shape setters, "
				"unless it is attached to any body");
		    	m_radius = r;
		    	needsRecreation = true;
		}

		void setCapsuleRadius(const float r)
		{
		    	CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
		    		"You are not allowed to use shape setters, "
		    		"unless it is attached to any body");
		    	m_radius = r;
		    	needsRecreation = true;
		}

		void setCapsuleHeight(const float h)
		{
		    	CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
		    		"You are not allowed to use shape setters, "
		    		"unless it is attached to any body");
		    	m_height = h;
		    	needsRecreation = true;
		}

		void setCylinderRadius(const float r)
		{
		    	CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
		    		"You are not allowed to use shape setters, "
		    		"unless it is attached to any body");
		    	m_radius = r;
		    	needsRecreation = true;
		}

		void setCylinderHeight(const float h)
		{
		    	CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId),
		    		"You are not allowed to use shape setters, "
		    		"unless it is attached to any body");
		    	m_height = h;
		    	needsRecreation = true;
		}

		[[nodiscard]] bool operator==(const Shape& otherShape) const
		{
			CLZ_ASSERT(
				B3_IS_NON_NULL(otherShape.getShapeId()) &&
					B3_IS_NON_NULL(m_shapeId),
				"comparing shape with a null shape id");

			return B3_ID_EQUALS(
				m_shapeId,
				otherShape.getShapeId());
		}

		bool isAttachedToSameBody(const Shape& otherShape) const
		{
			CLZ_ASSERT(
				B3_IS_NON_NULL(m_shapeId) &&
					B3_IS_NON_NULL(otherShape.getShapeId()),
				"comparing shape bodies with a null body id");

			return B3_ID_EQUALS(
				b3Shape_GetBody(m_shapeId),
				otherShape.getAttachedBodyId());
		}

	};


} // namespace clz::physics