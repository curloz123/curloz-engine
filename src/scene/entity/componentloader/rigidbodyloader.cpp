/**
 * @file rigidbodyloader.cpp
 * @brief Loads/Save all physics data of rigid bodies from scene.json
 * in entity table.
 * currently loads -
 * Body type - "static", "dynamic", "kinematic"
 * Position is retrieved from entity's transform component
 * linear/angular damping - "lineardamping" / "angulardamping"
 * Enable sleep?? - "enablesleep"
 * Linear/Angular locks - "linearlock" / "angularlock"
 * Box Shapes - "boxshapes {
 * Density - "density"
 * Restitution - "restitution"
 * Friction - "friction"
 * }
 * And finally creates and return BodyComponent and ShapeComponent
 *
 * @note - TransformComponent must be present
 */
#include "core/logs.hpp"
#include "entity/componentmanager.hpp"
#include "scene/entity/loader.hpp"

namespace clz::scene
{
	/// @copydoc retrieveBodyComponent
	physics::RigidBodyComponent
	retrieveBodyComponent(
			const nlohmann::json& physicsTable,
			const ecs::entity& entity,
			std::string_view entityName)
	{
		physics::BodyData data{};

		if (physicsTable.contains("type"))
		{
			const auto type = physicsTable["type"].get<std::string>();
			if (type == "static")
				data.type = physics::BodyType::StaticBody;
			else if (type == "dynamic")
				data.type = physics::BodyType::DynamicBody;
			else if (type == "kinematic")
				data.type = physics::BodyType::KinematicBody;
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "' does not "
					"have a valid physics:bodytype, assigning it dynamic by "
					"default"
				);
				data.type = physics::BodyType::DynamicBody;
			}
		}
		else
		{
			clz::log::warn(
				"entity '" + std::string(entityName) + "' does not "
				"have physics:bodytype entry, assigning it dynamic by "
				"default"
			);
			data.type = physics::BodyType::DynamicBody;
		}

		const auto& tc = ecs::getComponent<ecs::TransformComponent>(entity);

		data.position = tc.position;
		data.rotation = tc.rotation;

		if (physicsTable.contains("lineardamping"))
		{
			data.linearDamping = physicsTable["lineardamping"].get<float>();
		}
		else
		{
			clz::log::warn(
				"entity '" + std::string(entityName) + "' does not specify "
				"physics:lineardamping, assigning it 0 by default"
			);
			data.linearDamping = 0.0f;
		}

		if (physicsTable.contains("angulardamping"))
		{
			data.angularDamping = physicsTable["angulardamping"].get<float>();
		}
		else
		{
			clz::log::warn(
				"entity '" + std::string(entityName) + "' does not specify "
				"physics:angulardamping, assigning it 0.1 by default"
			);
			data.angularDamping = 0.1f;
		}

		if (physicsTable.contains("enablesleep"))
		{
			data.enableSleep = physicsTable["enablesleep"].get<bool>();
		}
		else
		{
			clz::log::warn(
				"entity '" + std::string(entityName) + "' does not specify "
				"physics:enablesleep, assigning it true by default"
			);
			data.enableSleep = true;
		}

		if (physicsTable.contains("linearlock"))
		{
			data.linearLocks = {
				physicsTable["linearlock"][0],
				physicsTable["linearlock"][1],
				physicsTable["linearlock"][2],
			};
		}
		else
		{
			clz::log::warn(
				"entity '" + std::string(entityName) + "' does not specify "
				"physics:linearlock, assigning all axes unlocked by default"
			);
			data.linearLocks = {false, false, false};
		}

		if (physicsTable.contains("angularlock"))
		{
			data.angularLocks = {
				physicsTable["angularlock"][0],
				physicsTable["angularlock"][1],
				physicsTable["angularlock"][2],
			};
		}
		else
		{
			clz::log::warn(
				"entity '" + std::string(entityName) + "' does not specify "
				"physics:angularlock, assigning all axes unlocked by default"
			);
			data.angularLocks = {false, false, false};
		}

		std::vector<physics::ShapeDef> ShapeDefs = {};
		auto loadShape = [&](const nlohmann::json& shape) {
			float density;
			if (shape.contains("density"))
			{
				density = shape["density"];
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': shape does "
					"not have density, assigning it default value: 10.0f"
				);
				density = 10.0f;
			}

			float friction;
			if (shape.contains("friction"))
			{
				friction = shape["friction"];
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': shape does "
					"not have friction, assigning it default value: 0.7f"
				);
				friction = 0.67f;
			}

			float restitution;
			if (shape.contains("restitution"))
			{
				restitution = shape["restitution"];
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': shape does "
					"not have restitution, assigning it default value: 0.5f"
				);
				restitution = 0.5f;
			}

			const math::vec3 position = {
				shape["position"][0],
				shape["position"][1],
				shape["position"][2]
			};

			const math::vec3 rotation = {
				shape["rotation"][0],
				shape["rotation"][1],
				shape["rotation"][2]
			};

			// NOTE: these three are only meaningfully filled in for the
			// branch matching their shape type below. They're zero-inited
			// here (rather than left uninitialized) since ShapeDef's
			// constructor takes all of them regardless of shapeType, and
			// reading an uninitialized float even when unused is UB.
			math::vec3 halfExtents{0.0f, 0.0f, 0.0f};
			float radius = 0.0f;
			float height = 0.0f;
			physics::ShapeType shapeType;

			const std::string type = shape["type"];
			if (type == "box")
			{
				shapeType = physics::ShapeType::BOX;
				CLZ_ASSERT(
					shape.contains("halfdimensions"),
					"corrupted scene file: entity '" + std::string(entityName) +
					"' lacks half dimensions entry in a box shape field"
				);
				halfExtents = {
					shape["halfdimensions"][0],
					shape["halfdimensions"][1],
					shape["halfdimensions"][2]
				};
			}
			else if (type == "sphere")
			{
				shapeType = physics::ShapeType::SPHERE;
				CLZ_ASSERT(
					shape.contains("radius"),
					"corrupted scene file: entity '" + std::string(entityName) +
					"' lacks radius entry in a sphere shape field"
				);
				radius = shape["radius"];
			}
			else if (type == "cylinder")
			{
				shapeType = physics::ShapeType::CYLINDER;
				CLZ_ASSERT(
					shape.contains("radius") && shape.contains("height"),
					"corrupted scene file: entity '" + std::string(entityName) +
					"' lacks radius or height entry in a cylinder shape field"
				);
				radius = shape["radius"];
				height = shape["height"];
			}
			else if (type == "capsule")
			{
				shapeType = physics::ShapeType::CAPSULE;
				CLZ_ASSERT(
					shape.contains("radius") && shape.contains("height"),
					"corrupted scene file: entity '" + std::string(entityName) +
					"' lacks radius or height entry in a capsule shape field"
				);
				radius = shape["radius"];
				height = shape["height"];
			}
			else
			{
				clz::log::warn(
					"Unknown shape type present in scene for entity '" +
					std::string(entityName) + "'"
				);
			}

			return physics::ShapeDef(
				shapeType,
				position,
				rotation,
				density,
				friction,
				restitution,
				halfExtents,
				radius,
				height
			);
		};

		if (physicsTable.contains("shapes"))
		{
			const auto& shapesTable = physicsTable["shapes"];
			std::vector<physics::ShapeDef> shapeDefs;
			shapeDefs.reserve(shapesTable.size());
			for (size_t i = 0; i < shapesTable.size(); ++i)
			{
				shapeDefs.emplace_back(loadShape(shapesTable[i]));
			}
			data.ShapeDefs
				.insert(data.ShapeDefs.end(), shapeDefs.begin(), shapeDefs.end());
		}

		physics::RigidBodyComponent rigidBodyComponent(
			physics::createBody(data),
			tc.rotation,
			tc.rotation,
			tc.position,
			tc.position
		);

		return rigidBodyComponent;
	}

	/// @copydoc saveRigidBodyComponent
	void saveRigidBodyComponent(
		physics::RigidBodyComponent rigidBodyComponent,
		nlohmann::json& physicsTable,
		std::string_view entityName
	)
	{
		const auto bodyId = rigidBodyComponent.rigidBodyId;

		const auto& type = physics::getBodyType(bodyId);
		switch (type)
		{
		case physics::BodyType::StaticBody:
			physicsTable["type"] = "static";
			break;
		case physics::BodyType::DynamicBody:
			physicsTable["type"] = "dynamic";
			break;
		case physics::BodyType::KinematicBody:
			physicsTable["type"] = "kinematic";
			break;
		default:
			log::error(
				"Invalid body type when saving scene.json for entity '" +
				std::string(entityName) + "': body[type]"
			);
			break;
		}

		physicsTable["lineardamping"] = physics::getBodyLinearDamping(bodyId);
		physicsTable["angulardamping"] = physics::getBodyAngularDamping(bodyId);
		physicsTable["enablesleep"] = physics::isSleepEnabled(bodyId);

		const auto& linearLocks = physics::getBodyLinearLocks(bodyId);
		physicsTable["linearlock"] = {
			linearLocks[0],
			linearLocks[1],
			linearLocks[2],
		};
		const auto& angularLocks = physics::getBodyAngularLocks(bodyId);
		physicsTable["angularlock"] = {
			angularLocks[0],
			angularLocks[1],
			angularLocks[2],
		};

		const auto& shapes = physics::getBodyShapes(bodyId);
		auto& shapesTable = physicsTable["shapes"];
		for (const auto& shape : shapes)
		{
			if (shape.isItTimeSon())
				continue;

			nlohmann::json shapeEntry;

			const auto position = shape.getPosition();
			shapeEntry["position"] = {position.x, position.y, position.z};

			const auto rotation = shape.getRotation();
			shapeEntry["rotation"] = {rotation.x, rotation.y, rotation.z};

			shapeEntry["density"] = shape.getDensity();
			shapeEntry["friction"] = shape.getFriction();
			shapeEntry["restitution"] = shape.getRestitution();

			switch (shape.getShapeType())
			{
			case physics::ShapeType::BOX: {
				shapeEntry["type"] = "box";
				const auto halfDimensions = shape.getBoxHalfExtents();
				shapeEntry["halfdimensions"] = {
					halfDimensions.x,
					halfDimensions.y,
					halfDimensions.z
				};
				break;
			}
			case physics::ShapeType::SPHERE: {
				shapeEntry["type"] = "sphere";
				shapeEntry["radius"] = shape.getSphereRadius();
				break;
			}
			case physics::ShapeType::CAPSULE: {
				shapeEntry["type"] = "capsule";
				shapeEntry["radius"] = shape.getCapsuleRadius();
				shapeEntry["height"] = shape.getCapsuleHeight();
				break;
			}
			case physics::ShapeType::CYLINDER: {
				shapeEntry["type"] = "cylinder";
				shapeEntry["radius"] = shape.getCylinderRadius();
				shapeEntry["height"] = shape.getCylinderHeight();
				break;
			}

			default:
				log::warn(
					"Unknown shape type for entity '" +
					std::string(entityName) + "'"
				);
			}

			shapesTable.push_back(shapeEntry);
		}
	}

} // namespace clz::scene
