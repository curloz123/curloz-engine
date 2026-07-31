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
#include "scene/entity/loader.hpp"
#include "entity/componentmanager.hpp"

namespace clz::scene
{
	/// @brief Loads all body and shapes from JSON
	/// @param physicsTable Physics table in JSON file
	/// @param entity Entity for which we are creating this entity
	/// @return std::tuple<BodyComponent, ShapeComponent> Both components
	/// @note if a value is not present in JSON, will assign default value
	ecs::RigidBodyComponent retrieveBodyComponent(
		const nlohmann::json& physicsTable,
		const ecs::entity& entity)
	{
		physics::BodyData data{};

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
					"entity entry does not specify physics:bodytype,"
					"assigning it dynamic by default");
				data.type = physics::BodyType::DynamicBody;
			}
		}

		const auto& tc =
			ecs::getComponent<ecs::TransformComponent>(entity);

		data.position = tc.position;
		data.rotation = tc.rotation;

		if (physicsTable.contains("lineardamping"))
		{
			data.linearDamping = physicsTable["lineardamping"].get<float>();
		}
		else
		{
			clz::log::warn(
				"entity entry does not specify physics:lineardamping, "
				"assigning it 0 by default");
			data.linearDamping = 0.0f;
		}

		if (physicsTable.contains("angulardamping"))
		{
			data.angularDamping = physicsTable["angulardamping"].get<float>();
		}
		else
		{
			clz::log::warn(
				"entity entry does not specify physics:angulardamping, "
				"assigning it 0.1 by default");
			data.angularDamping = 0.1f;
		}

		if (physicsTable.contains("enablesleep"))
		{
			data.enableSleep = physicsTable["enablesleep"].get<bool>();
		}
		else
		{
			clz::log::warn(
				"entity entry does not specify physics:enablesleep, "
				"assigning it true by default");
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
				"entity entry does not specify physics:linearlock, "
				"assigning all axes unlocked by default");
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
				"entity entry does not specify physics:angularlock, "
				"assigning all axes unlocked by default");
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
					"Shape does not have density, "
					"assigning it default value: 10.0f");
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
					"Shape does not have friction, "
					"assigning it default value: 0.7f");
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
					"Shape does not have restitution, "
					"assigning it default value: 0.5f");
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


			math::vec3 halfExtents;
			float radius;
			float height;
			physics::ShapeType shapeType;

			const std::string type = shape["type"];
			if (type == "box")
			{
				shapeType = physics::ShapeType::BOX;
				CLZ_ASSERT(
					shape.contains("halfdimensions"),
					"corrupted scene file"
					"lacks half dimensions entry in a box shape field");
				halfExtents = {
					shape["halfdimensions"][0],
					shape["halfdimensions"][1],
					shape["halfdimensions"][2] };
			}
			else if (type == "sphere")
			{
				shapeType = physics::ShapeType::SPHERE;
				CLZ_ASSERT(
					shape.contains("radius"),
					"corrupted scene file"
					"lacks radius entry in a sphere shape field");
				radius = shape["radius"];
			}
			else if (type == "cylinder")
			{
				shapeType = physics::ShapeType::CYLINDER;
				CLZ_ASSERT(
					shape.contains("radius") &&
						shape.contains("height"),
					"corrupted scene file"
					"lacks radius or height entry in a cylinder shape field");
				radius = shape["radius"];
				height = shape["height"];
			}
			else if (type == "capsule")
			{
				shapeType = physics::ShapeType::CAPSULE;
				CLZ_ASSERT(
					shape.contains("radius") &&
						shape.contains("height"),
					"corrupted scene file"
					"lacks radius or height entry in a capsule shape field");
				radius = shape["radius"];
				height = shape["height"];
			}
			else
			{
				CLZ_ASSERT(false, "Unknown shape type present in scene");
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
				height);
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
			data.ShapeDefs.insert(
				data.ShapeDefs.end(),
				shapeDefs.begin(),
				shapeDefs.end());
		}

		ecs::RigidBodyComponent rigidBodyComponent(
			physics::createBody(data),
			tc.rotation,
			tc.rotation,
			tc.position,
			tc.position);

		return rigidBodyComponent;
	}

	/// @brief Saves back all physics data of entities to JSON
	/// @param rigidBodyComponent Tuple containing both BodyComponent and ShapeComponent of entity
	/// @param physicsTable JSON-array where we have to write back data
	void saveRigidBodyComponent(
		const ecs::RigidBodyComponent rigidBodyComponent,
		nlohmann::json& physicsTable)
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
			log::error("Invalid body type when saving scene.json: body[type]");
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
		for (size_t i = 0; i < shapes.size(); ++i)
		{
			if (shapes[i].isItTimeSon())
				continue;

			const auto position = shapes[i].getPosition();
			shapesTable[i]["position"][0] = position.x;
			shapesTable[i]["position"][1] = position.y;
			shapesTable[i]["position"][2] = position.z;

			const auto rotation = shapes[i].getRotation();
			shapesTable[i]["rotation"][0] = rotation.x;
			shapesTable[i]["rotation"][1] = rotation.y;
			shapesTable[i]["rotation"][2] = rotation.z;

			shapesTable[i]["density"] = shapes[i].getDensity();
			shapesTable[i]["friction"] = shapes[i].getFriction();
			shapesTable[i]["restitution"] = shapes[i].getRestitution();

			switch (shapes[i].getShapeType())
			{
			case physics::ShapeType::BOX: {

				shapesTable[i]["type"] = "box";
				const auto halfDimensions = shapes[i].getBoxHalfExtents();
				shapesTable[i]["halfdimensions"][0] = halfDimensions.x;
				shapesTable[i]["halfdimensions"][1] = halfDimensions.y;
				shapesTable[i]["halfdimensions"][2] = halfDimensions.z;
				break;
			}

			case physics::ShapeType::SPHERE:{
				shapesTable[i]["type"] = "sphere";
				const auto radius = shapes[i].getSphereRadius();
				shapesTable[i]["radius"] = radius;
				break;
			}

			case physics::ShapeType::CAPSULE: {
				shapesTable[i]["type"] = "capsule";
				const auto radius = shapes[i].getCapsuleRadius();
				const auto height = shapes[i].getCapsuleHeight();
				shapesTable[i]["radius"] = radius;
				shapesTable[i]["height"] = height;
				break;
			}
			case physics::ShapeType::CYLINDER: {
				shapesTable[i]["type"] = "cylinder";
				const auto radius = shapes[i].getCylinderRadius();
				const auto height = shapes[i].getCylinderHeight();
				shapesTable[i]["radius"] = radius;
				shapesTable[i]["height"] = height;
				break;
			}
			}
		}
	}

} // namespace clz::ecs