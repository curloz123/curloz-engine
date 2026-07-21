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
 * Box Shapes - "boxshapes{
 * Density - "density"
 * Restitution - "restitution"
 * Friction - "friction"
 * }
 * And finally creates and return BodyComponent and ShapeComponent
 *
 * @note - TransformComponent must be present
 */
#include "core/logs.hpp"
#include "scene/entity/componentloader/loader.hpp"
#include "scene/entity/componentmanager.hpp"

namespace clz::ecs
{
	/// @brief Loads all body and shapes from JSON
	/// @param physicsTable Physics table in JSON file
	/// @param entity Entity for which we are creating this entity
	/// @return std::tuple<BodyComponent, ShapeComponent> Both components
	/// @note if a value is not present in JSON, will assign default value
	std::tuple<BodyComponent, ShapeComponent> retrieveBodyComponent(const nlohmann::json& physicsTable, const entity& entity)
	{
		physics::BodyData data{};

		const auto type = physicsTable["type"].get<std::string>();
		if (type == "static")
			data.type = physics::BodyType::StaticBody;
		else if (type == "dynamic")
			data.type = physics::BodyType::DynamicBody;
		else if (type == "kinematic")
			data.type = physics::BodyType::KinematicBody;
		else
		{
			clz::log::warn("entity entry does not specify physics:bodytype,"
				       "assigning it dynamic by default");
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
			clz::log::warn("entity entry does not specify physics:lineardamping, "
				       "assigning it 0 by default");
			data.linearDamping = 0.0f;
		}

		if (physicsTable.contains("angulardamping"))
		{
			data.angularDamping = physicsTable["angulardamping"].get<float>();
		}
		else
		{
			clz::log::warn("entity entry does not specify physics:angulardamping, "
				       "assigning it 0.1 by default");
			data.angularDamping = 0.1f;
		}

		if (physicsTable.contains("enablesleep"))
		{
			data.enableSleep = physicsTable["enablesleep"].get<bool>();
		}
		else
		{
			clz::log::warn("entity entry does not specify physics:enablesleep, "
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
			clz::log::warn("entity entry does not specify physics:linearlock, "
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
			clz::log::warn("entity entry does not specify physics:angularlock, "
				       "assigning all axes unlocked by default");
			data.angularLocks = {false, false, false};
		}

		std::vector<physics::BoxShape> boxShapes = {};
		auto loadBoxShape = [&](const nlohmann::json& shape) {
			physics::BoxShape boxShape;

			if (shape.contains("density"))
			{
				boxShape.density = shape["density"];
			}
			else
			{
				clz::log::warn("Shape does not have density, "
					       "assigning it default value: 10.0f");
				boxShape.density = 10.0f;
			}
			if (shape.contains("friction"))
			{
				boxShape.friction = shape["friction"];
			}
			else
			{
				clz::log::warn("Shape does not have friction, "
					       "assigning it default value: 0.7f");
				boxShape.friction = 0.67f;
			}
			if (shape.contains("restitution"))
			{
				boxShape.restitution = shape["restitution"];
			}
			else
			{
				clz::log::warn("Shape does not have restitution, "
					       "assigning it default value: 0.5f");
				boxShape.restitution = 0.5f;
			}

			boxShape.halfDimensions.x = shape["halfdimensions"][0];
			boxShape.halfDimensions.y = shape["halfdimensions"][1];
			boxShape.halfDimensions.z = shape["halfdimensions"][2];

			boxShape.position.x = shape["position"][0];
			boxShape.position.y = shape["position"][1];
			boxShape.position.z = shape["position"][2];

			boxShape.rotation.x = shape["rotation"][0];
			boxShape.rotation.y = shape["rotation"][1];
			boxShape.rotation.z = shape["rotation"][2];

			boxShapes.emplace_back(boxShape);
		};

		if (physicsTable.contains("boxshapes"))
		{
			const auto& shapesTable = physicsTable["boxshapes"];
			for (size_t i = 0; i < shapesTable.size(); ++i)
			{
				loadBoxShape(shapesTable[i]);
			}
			data.boxShapes = boxShapes;
		}
		else
		{
			data.boxShapes = {};
		}

		BodyComponent bodyComponent(physics::createBody(data), tc.rotation, tc.rotation, tc.position, tc.position);
		std::vector<physics::BoxShape> boxShapesContainer = {};
		for (auto& boxShape : boxShapes)
		{
			physics::attachShapeToBody(bodyComponent.bodyId, boxShapesContainer, boxShape);
		}
		ShapeComponent shapeComponent(boxShapesContainer);

		return std::make_tuple(bodyComponent, shapeComponent);
	}

	/// @brief Saves back all physics data of entities to JSON
	/// @param rigidBodyComponent Tuple containing both BodyComponent and ShapeComponent of entity
	/// @param physicsTable JSON-array where we have to write back data
	void saveRigidBodyComponent(const std::tuple<BodyComponent, ShapeComponent>& rigidBodyComponent, nlohmann::json& physicsTable)
	{
		const auto& [bodyComponent, shapeComponent] = rigidBodyComponent;
		const auto& bodyId = bodyComponent.bodyId;
		const auto& boxShapes = shapeComponent.boxShapes;

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

		auto& shapesTable = physicsTable["boxshapes"];
		for (size_t i = 0; i < boxShapes.size(); ++i)
		{
			shapesTable[i]["density"] = boxShapes[i].density;
			shapesTable[i]["friction"] = boxShapes[i].friction;
			shapesTable[i]["restitution"] = boxShapes[i].restitution;

			shapesTable[i]["halfdimensions"][0] = boxShapes[i].halfDimensions.x;
			shapesTable[i]["halfdimensions"][1] = boxShapes[i].halfDimensions.y;
			shapesTable[i]["halfdimensions"][2] = boxShapes[i].halfDimensions.z;

			shapesTable[i]["position"][0] = boxShapes[i].position.x;
			shapesTable[i]["position"][1] = boxShapes[i].position.y;
			shapesTable[i]["position"][2] = boxShapes[i].position.z;

			shapesTable[i]["rotation"][0] = boxShapes[i].rotation.x;
			shapesTable[i]["rotation"][1] = boxShapes[i].rotation.y;
			shapesTable[i]["rotation"][2] = boxShapes[i].rotation.z;
		}
	}

} // namespace clz::ecs