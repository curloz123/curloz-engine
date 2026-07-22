/**
 * @file shape.cpp
 * @author curl0z
 * @brief Physics shape implementation file
 */
#include "physics/shape.hpp"
#include "core/assert.hpp"
#include "math/quateulerconv.hpp"
#include "physics/math.hpp"
#include "entity/components.hpp"

namespace clz::physics
{
	/**
	 * @brief Attaches shape to a body
	 * @param bodyId ID of body to which we're attaching the shape
	 * @param boxShapeContainer Container which holds all shapes of the body
	 * @param shape The actual shape data
	 */
	void attachShapeToBody(const b3BodyId& bodyId, std::vector<BoxShape>& boxShapeContainer, BoxShape& shape)
	{
		const b3Vec3 pos = toVec3(shape.position);
		const b3Quat quat = toQuat(math::quatFromEuler(shape.rotation));
		const b3Transform localTransform = {pos, quat};
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = shape.density;
		shapeDef.baseMaterial.friction = shape.friction;
		shapeDef.baseMaterial.restitution = shape.restitution;
		const b3BoxHull cuboid =
		    b3MakeTransformedBoxHull(shape.halfDimensions.x, shape.halfDimensions.y, shape.halfDimensions.z, localTransform);

		shape.shapeId = b3CreateHullShape(bodyId, &shapeDef, &cuboid.base);
		b3Body_ApplyMassFromShapes(bodyId);
		boxShapeContainer.emplace_back(shape);
	}

	/**
	 * @brief Modify's an existing shape of a body
	 * @param bodyId Id of body whose shape we are modifying
	 * @param shape New Shape data
	 * @param boxShapeContainer Container which holds the body's shapes, indexing is done on this container
	 * @param index Index of the shape which needs to be changed
	 */
	void modifyShapeByIndex(const b3BodyId& bodyId, BoxShape& shape, std::vector<BoxShape>& boxShapeContainer, uint32_t index)
	{
		CLZ_ASSERT(index < boxShapeContainer.size(), "Invalid index passed while modifying shape data");

		b3DestroyShape(boxShapeContainer[index].shapeId, true);
		const b3Vec3 pos = toVec3(shape.position);
		const b3Quat quat = toQuat(math::quatFromEuler(shape.rotation));
		const b3Transform localTransform = {pos, quat};
		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = shape.density;
		shapeDef.baseMaterial.friction = shape.friction;
		shapeDef.baseMaterial.restitution = shape.restitution;
		const b3BoxHull cuboid =
		    b3MakeTransformedBoxHull(shape.halfDimensions.x, shape.halfDimensions.y, shape.halfDimensions.z, localTransform);

		shape.shapeId = b3CreateHullShape(bodyId, &shapeDef, &cuboid.base);
		boxShapeContainer[index] = shape;
		b3Body_ApplyMassFromShapes(bodyId);
	}

	/**
	 * @param bodyId ID of body
	 * @param boxShapeContainer Container holding all the shapes for this body
	 * @param shapeId ID of shape
	 */
	void destroyShape(const b3BodyId& bodyId, std::vector<BoxShape>& boxShapeContainer, b3ShapeId& shapeId)
	{
		b3DestroyShape(shapeId, true);

		/* TO-DO
		size_t i = 0;
		for (; i < boxShapeContainer.size(); i++)
		{
			if (boxShapeContainer[i].shapeId == shapeId)
			{

			}
		}
		*/
	}
} // namespace clz::physics