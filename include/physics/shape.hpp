/**
@file shape.hpp
@author curl0z
@brief Physics shape header file.
Provides BoxShape Data-Structure which holds all the data of any shape attached to a body.
*/
#pragma once
#include "core/assert.hpp"
#include "math/quateulerconv.hpp"
#include "math/vec3.hpp"
#include "physics.hpp"
#include "core/logs.hpp"

namespace clz::physics
{
	/// @brief Enumeration of supported primitive shape types.
	enum class ShapeType
	{
		BOX,	 ///< @brief Box/Cuboid shape.
		SPHERE,	 ///< @brief Sphere shape.
		CAPSULE, ///< @brief Capsule shape.
		CYLINDER ///< @brief Cylinder shape.
	};

	/// @brief Definition data for creating a physics shape.
	struct ShapeDef
	{
		ShapeType shapeType;		///< @brief The type of the shape.
		math::vec3 position;		///< @brief Local position relative to the body.
		math::vec3 rotation;		///< @brief Local rotation (Euler angles) relative to the body.
		float density;			///< @brief Density of the shape (used for mass calculation).
		float friction;			///< @brief Friction coefficient.
		float restitution;		///< @brief Restitution (bounciness) coefficient.
		math::vec3 halfExtents; 	///< @brief Half-extents for BOX shape.
		float radius;			///< @brief Radius for SPHERE, CAPSULE, CYLINDER shapes.
		float height;			///< @brief Height for CAPSULE, CYLINDER shapes.
		bool shouldBeDestroyed = false; ///< @brief Hints whether shape should be destroyed
		ShapeDef()
		{
		}

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

		bool operator==(const ShapeDef& shapeDef) const
		{
			return  shapeType == shapeDef.shapeType &&
				position == shapeDef.position && rotation == shapeDef.rotation &&
				density == shapeDef.density && friction == shapeDef.friction &&
				restitution == shapeDef.restitution &&
				halfExtents == shapeDef.halfExtents &&
				radius == shapeDef.radius && height == shapeDef.height &&
				shouldBeDestroyed == shapeDef.shouldBeDestroyed;
		}
		bool operator!=(const ShapeDef& shapeDef) const
		{
			return !(*this == shapeDef);
		}
	};

	/// @brief Wrapper class for a Box3D shape, managing its lifecycle and properties.
	class Shape
	{
	private:
		b3ShapeId m_shapeId;		///< @brief Internal Box3D shape handle.
		ShapeType m_shapeType;		///< @brief The primitive type of the shape.
		math::vec3 m_position;		///< @brief Local position offset.
		math::vec3 m_rotation;		///< @brief Local rotation offset.
		math::vec3 m_halfExtents;	///< @brief Half-extents (for BOX).
		float m_radius;			///< @brief Radius (for SPHERE, CAPSULE, CYLINDER).
		float m_height;			///< @brief Height (for CAPSULE, CYLINDER).
		bool needsRecreation = false;	///< @brief Flag indicating if shape properties changed and require rebuilding.
		bool shouldBeDestroyed = false; ///< @brief Flag indicating if the shape is marked for permanent deletion.

	public:
		/// @brief Default constructor.
		Shape()
		{
		}

		/// @brief Constructs a shape and attaches it to a body.
		/// @param shapeDef The shape definition.
		/// @param rigidBodyId The ID of the body to attach to.
		Shape(const ShapeDef& shapeDef, RigidBodyId rigidBodyId);

		/// @brief Creates the underlying Box3D shape.
		/// @param shapeDef The shape definition.
		/// @param rigidBodyId The ID of the body to attach to.
		void createShape(const ShapeDef& shapeDef, RigidBodyId rigidBodyId);

		/// @brief Destroys the underlying Box3D shape.
		/// @param isRecreating If true, marks the shape for recreation rather than permanent deletion.
		void destroyShape(bool isRecreating = false);

		/// @brief Recreates the shape (e.g., after property changes that require rebuilding).
		/// @param rigidBodyId The ID of the attached body.
		void recreateShape(RigidBodyId rigidBodyId);

		/// @brief Retrieves the complete definition data of the shape.
		/// @return A ShapeDef struct populated with current shape properties.
		ShapeDef getShapeData() const
		{
			return ShapeDef(this->getShapeType(), this->getPosition(), this->getRotation(), this->getDensity(), this->getFriction(),
					this->getRestitution(), this->m_halfExtents, this->m_radius, this->m_height);
		}

		/// @brief Checks if the shape's properties have been modified and require recreation.
		/// @return True if the shape is outdated and needs recreation.
		bool isOutdated() const
		{
			return needsRecreation;
		}

		void logData() const
		{
			clz::log::info("density: " + std::to_string(this->getDensity()));
			clz::log::info("mass: " + std::to_string(this->getDensity()));
			clz::log::info("density: " + std::to_string(this->getRestitution()));
			clz::log::info("density: " + std::to_string(this->getFriction()));
			clz::log::info("density: " + std::to_string(this->getDensity()));
			clz::log::info("density: " + std::to_string(this->getDensity()));
		}

		/// @brief Checks if the shape is marked for permanent destruction.
		/// @return True if the shape should be destroyed.
		bool isItTimeSon() const
		{
			return shouldBeDestroyed;
		}

		/// @brief Gets the shape type.
		/// @return The ShapeType enum value.
		ShapeType getShapeType() const
		{
			return m_shapeType;
		}

		/// @brief Gets the ID of the body this shape is attached to.
		/// @return The b3BodyId handle.
		b3BodyId getAttachedBodyId() const
		{
			return b3Shape_GetBody(m_shapeId);
		}

		/// @brief Gets the internal Box3D shape ID.
		/// @return The b3ShapeId handle.
		b3ShapeId getShapeId() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "attempt to call shape getter on a null shape");
			return m_shapeId;
		}

		/// @brief Gets the friction coefficient.
		/// @return The friction value.
		float getFriction() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters,"
							      "unless it is attached to any body");
			return b3Shape_GetFriction(m_shapeId);
		}

		/// @brief Gets the restitution (bounciness) coefficient.
		/// @return The restitution value.
		float getRestitution() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters,"
							      "unless it is attached to any body");
			return b3Shape_GetRestitution(m_shapeId);
		}

		/// @brief Gets the density of the shape.
		/// @return The density value.
		float getDensity() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters,"
							      "unless it is attached to any body");
			return b3Shape_GetDensity(m_shapeId);
		}

		/// @brief Gets the local position offset.
		/// @return The position vector.
		math::vec3 getPosition() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters,"
							      "unless it is attached to any body");
			return m_position;
		}

		/// @brief Gets the local rotation offset.
		/// @return The rotation vector (Euler).
		math::vec3 getRotation() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters,"
							      "unless it is attached to any body");
			return m_rotation;
		}

		/// @brief Sets the friction coefficient.
		/// @param friction New friction value.
		void setFriction(const float friction)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters,"
							      "unless it is attached to any body");
			b3Shape_SetFriction(m_shapeId, friction);
		}

		/// @brief Sets the restitution coefficient.
		/// @param restitution New restitution value.
		void setRestitution(const float restitution)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters,"
							      "unless it is attached to any body");
			b3Shape_SetRestitution(m_shapeId, restitution);
		}

		/// @brief Sets the density of the shape.
		/// @param density New density value.
		void setDensity(const float density)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters,"
							      "unless it is attached to any body");
			b3Shape_SetDensity(m_shapeId, density, true);
		}

		/// @brief Sets the local position offset.
		/// @note Marks the shape as needing recreation.
		/// @param pos New position vector.
		void setPosition(const math::vec3 pos)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters,"
							      "unless it is attached to any body");
			m_position = pos;
			needsRecreation = true;
		}

		/// @brief Sets the local rotation offset.
		/// @note Marks the shape as needing recreation.
		/// @param rot New rotation vector (Euler).
		void setRotation(const math::vec3 rot)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters,"
							      "unless it is attached to any body");
			m_rotation = rot;
			needsRecreation = true;
		}

		/// @brief Gets the half-extents of a BOX shape.
		/// @return The half-extents vector.
		math::vec3 getBoxHalfExtents() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters,"
							      "unless it is attached to any body");
			return m_halfExtents;
		}

		/// @brief Gets the radius of a SPHERE shape.
		/// @return The radius value.
		float getSphereRadius() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters,"
							      "unless it is attached to any body");
			return m_radius;
		}

		/// @brief Gets the radius of a CAPSULE shape.
		/// @return The radius value.
		float getCapsuleRadius() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters, "
							      "unless it is attached to any body");
			return m_radius;
		}

		/// @brief Gets the height of a CAPSULE shape.
		/// @return The height value.
		float getCapsuleHeight() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters, "
							      "unless it is attached to any body");
			return m_height;
		}

		/// @brief Gets the radius of a CYLINDER shape.
		/// @return The radius value.
		float getCylinderRadius() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters, "
							      "unless it is attached to any body");
			return m_radius;
		}

		/// @brief Gets the height of a CYLINDER shape.
		/// @return The height value.
		float getCylinderHeight() const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape getters, "
							      "unless it is attached to any body");
			return m_height;
		}

		/// @brief Sets the half-extents of a BOX shape.
		/// @note Marks the shape as needing recreation.
		/// @param extents New half-extents vector.
		void setBoxHalfExtents(const math::vec3& extents)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters, "
							      "unless it is attached to any body");
			m_halfExtents = extents;
			needsRecreation = true;
		}

		/// @brief Sets the radius of a SPHERE shape.
		/// @note Marks the shape as needing recreation.
		/// @param r New radius value.
		void setSphereRadius(const float r)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters, "
							      "unless it is attached to any body");
			m_radius = r;
			needsRecreation = true;
		}

		/// @brief Sets the radius of a CAPSULE shape.
		/// @note Marks the shape as needing recreation.
		/// @param r New radius value.
		void setCapsuleRadius(const float r)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters, "
							      "unless it is attached to any body");
			m_radius = r;
			needsRecreation = true;
		}

		/// @brief Sets the height of a CAPSULE shape.
		/// @note Marks the shape as needing recreation.
		/// @param h New height value.
		void setCapsuleHeight(const float h)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters, "
							      "unless it is attached to any body");
			m_height = h;
			needsRecreation = true;
		}

		/// @brief Sets the radius of a CYLINDER shape.
		/// @note Marks the shape as needing recreation.
		/// @param r New radius value.
		void setCylinderRadius(const float r)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters, "
							      "unless it is attached to any body");
			m_radius = r;
			needsRecreation = true;
		}

		/// @brief Sets the height of a CYLINDER shape.
		/// @note Marks the shape as needing recreation.
		/// @param h New height value.
		void setCylinderHeight(const float h)
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId), "You are not allowed to use shape setters, "
							      "unless it is attached to any body");
			m_height = h;
			needsRecreation = true;
		}

		/// @brief Equality operator comparing internal shape IDs.
		/// @param otherShape The shape to compare against.
		/// @return True if both shapes refer to the same underlying Box3D shape.
		[[nodiscard]] bool operator==(const Shape& otherShape) const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(otherShape.getShapeId()) && B3_IS_NON_NULL(m_shapeId), "comparing shape with a null shape id");
			return B3_ID_EQUALS(m_shapeId, otherShape.getShapeId());
		}

		/// @brief Checks if two shapes are attached to the same body.
		/// @param otherShape The shape to compare against.
		/// @return True if attached to the same body.
		bool isAttachedToSameBody(const Shape& otherShape) const
		{
			CLZ_ASSERT(B3_IS_NON_NULL(m_shapeId) && B3_IS_NON_NULL(otherShape.getShapeId()),
				   "comparing shape bodies with a null body id");
			return B3_ID_EQUALS(b3Shape_GetBody(m_shapeId), otherShape.getAttachedBodyId());
		}
	};
} // namespace clz::physics