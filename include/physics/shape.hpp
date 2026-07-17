/**
 * @file shape.hpp
 * @author curl0z
 * @brief Physics shape header file
 * Provides BoxShape Data-Structure which holds
 * All the data of any shape attached to a body
 */
#pragma once

#include "math/vec3.hpp"
#include "physics.hpp"
#include "scene/entity/entitymanager.hpp"

namespace clz::physics
{
	/// @brief Cuboid Shape definition
	struct BoxShape
	{
		/// @brief Shape ID
		b3ShapeId shapeId;

		/// @brief Vector3 Half Dimensions of cuboid
		math::vec3 halfDimensions;

		/// @brief transform of this shape
		math::vec3 position;
		math::vec3 rotation;

		/// @brief shape properties
		float density;
		float friction;
		float restitution;

		BoxShape() : shapeId(b3_nullShapeId), halfDimensions(math::vec3(1.0f)), density(10.0f), friction(0.1f), restitution(0.5f)
		{
		}

		/**
		 * @brief Initializes cube shape
		 * @param halfDimensions Vector3 half dimensions of cuboid
		 * @param position local position of cube relative to its parent body
		 * @param rotation local euler rotation of cube relative to its parent body
		 * @param density density of shape, this parameter is necessary to apply
		 * @param friction friction of the shape
		 * @param restitution restitution value of the shape
		 */
		BoxShape(const math::vec3& halfDimensions, const math::vec3& position, const math::vec3& rotation, const float density,
			 const float friction, const float restitution)
		    : shapeId(b3_nullShapeId), halfDimensions(halfDimensions), position(position), rotation(rotation), density(density),
		      friction(friction), restitution(restitution)
		{
		}
	};

	/**
	 * @brief Attaches this shape to a given body
	 * @param bodyId Id of body to which this shape has to be attached
	 * @param boxShapeContainer container of body which holds the shapes
	 * @param shape The shape to attach
	 */
	void attachShapeToBody(const b3BodyId& bodyId, std::vector<BoxShape>& boxShapeContainer, BoxShape& shape);

	/**
	 * @brief Modifies a current existing shape
	 * @param bodyId Id of body whose shape has to be modified
	 * @param shape New shape
	 * @param boxShapeContainer container of body which holds the shapes
	 * @param index Index in provided array
	 */
	void modifyShapeByIndex(const b3BodyId& bodyId, BoxShape& shape, std::vector<BoxShape>& boxShapeContainer, uint32_t index);

	/**
	 * @brief Destroys shape of a body.
	 * Mass is updated too depending upon density of shape being removed.
	 * Also, this is not required to call this directly, all shapes of
	 * any body are destroyed when world is destroying all bodies.
	 * @param bodyId Id of body whose shape has to be removed
	 * @param boxShapeContainer container of body which holds the shapes
	 * @param shapeId ID of shape to be destroyed
	 */
	void destroyShape(const b3BodyId& bodyId, std::vector<BoxShape>& boxShapeContainer, b3ShapeId& shapeId);

} // namespace clz::physics