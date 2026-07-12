#include "scene/entity/components.hpp"
#include "math/quateulerconv.hpp"
#include "physics/math.hpp"
#include "scene/entity/entitymanager.hpp"
#include "scene/entity/componentmanager.hpp"
#include "physics/shape.hpp"

#include "core/assert.hpp"
#include "physics/physics.hpp"

namespace clz::physics
{
	void attachShapeToBody(const ecs::entity& entityId, BoxShape& shape)
	{
		const b3Vec3 pos = toVec3(shape.position);
		const b3Quat quat = toQuat(math::quatFromEuler(shape.rotation));
		const b3Transform localTransform = {pos, quat};
		const b3ShapeDef shapeDef = b3DefaultShapeDef();
		const b3BoxHull cuboid = b3MakeTransformedBoxHull(
			shape.halfDimensions.x, shape.halfDimensions.y, shape.halfDimensions.z, localTransform);

		const auto& rigidBody = ecs::getComponent<ecs::RigidBodyComponent>(entityId);
		shape.shapeId = b3CreateHullShape(rigidBody.bodyId, &shapeDef, &cuboid.base);

		auto& rigidBodyData = ecs::getComponent<ecs::RigidBodyDataComponent>(entityId);
		rigidBodyData.boxShapes.emplace_back(shape);
	}

	void modifyShapeByIndex(const ecs::entity& entityId, BoxShape& shape, const uint32_t index)
	{
		auto& rigidBodyData = ecs::getComponent<ecs::RigidBodyDataComponent>(entityId);
		clz::log::debug("shapes count" + std::to_string(rigidBodyData.boxShapes.size()));
		clz::log::debug("index: " + std::to_string(index));
		clz::CLZ_ASSERT(index <= rigidBodyData.boxShapes.size(),
			"Invalid index passed while modifying shape data");

		b3DestroyShape(rigidBodyData.boxShapes[index].shapeId, true);
		const b3Vec3 pos = toVec3(shape.position);
		const b3Quat quat = toQuat(math::quatFromEuler(shape.rotation));
		const b3Transform localTransform = {pos, quat};
		const b3ShapeDef shapeDef = b3DefaultShapeDef();
		const b3BoxHull cuboid = b3MakeTransformedBoxHull(
			shape.halfDimensions.x, shape.halfDimensions.y, shape.halfDimensions.z, localTransform);

		const auto& rigidBody = ecs::getComponent<ecs::RigidBodyComponent>(entityId);
		shape.shapeId = b3CreateHullShape(rigidBody.bodyId, &shapeDef, &cuboid.base);
		rigidBodyData.boxShapes[index] = shape;
	}


	void destroyShape(const ecs::entity& entityId, b3ShapeId& shapeId)
	{
		b3DestroyShape(shapeId, true);
	}
}