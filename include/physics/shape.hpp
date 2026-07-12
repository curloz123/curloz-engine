#pragma once

#include "math/vec3.hpp"
#include "scene/entity/entitymanager.hpp"
#include "physics.hpp"

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

		BoxShape() : shapeId(b3_nullShapeId), halfDimensions(math::vec3(1.0f))
		{
		}

		/**
		 * @brief Initializes cube shape
		 * @param halfDimensions Vector3 half dimensions of cuboid
		 * @param position local position of cube relative to its parent body
		 * @param rotation local euler rotation of cube relative to its parent body
		 */
		BoxShape(const math::vec3& halfDimensions,
			const math::vec3& position, const math::vec3& rotation) :
			shapeId(b3_nullShapeId), halfDimensions(halfDimensions),
			position(position), rotation(rotation)
		{
		}

	};


	/**
	 * @brief Attaches this shape to a given body
	 * @param entityId entity to which this shape has to be attached
	 * @param shape The shape to attach
	 */
	void attachShapeToBody(const ecs::entity& entityId, BoxShape& shape);

	void modifyShapeByIndex(const ecs::entity& entityId, BoxShape& shape, uint32_t index);

	/**
	 * @brief Destroys a shape.
	 * No need to pass BodyId specifically. Shape is removed automatically.
	 * And mass is updated too depending upon density of shape being removed.
	 * Also, this is not required to call this directly, all shapes of
	 * any body are destroyed when world is destroying all bodies.
	 * @param shapeId ID of shape to be destroyed
	 */
	void destroyShape(const ecs::entity& entityId, b3ShapeId& shapeId);
}