#include "scene/entity/componentloader/loader.hpp"
#include "core/logs.hpp"

namespace clz::ecs
{
	std::tuple<RigidBodyComponent, RigidBodyDataComponent> retrieveBodyComponent(const nlohmann::json& physicsTable,
									const TransformComponent& tc)
	{
		physics::BodyData data;

		data.mass = physicsTable["mass"].get<float>();
		const auto type = physicsTable["type"].get<std::string>();
		if (type == "static")
			data.type = physics::BodyType::StaticBody;
		else if (type == "dynamic")
			data.type = physics::BodyType::DynamicBody;
		else if (type == "kinematic")
			data.type = physics::BodyType::KinematicBody;
		else
			log::error("Invalid entry scene.json: body[type]");
		data.position = tc.position;
		data.rotation = tc.rotation;
		data.linearDamping = physicsTable["lineardamping"].get<float>();
		data.angularDamping = physicsTable["angulardamping"].get<float>();
		data.enableSleep = physicsTable["enablesleep"].get<bool>();
		data.linearLocks = {
			physicsTable["linearlock"][0],
			physicsTable["linearlock"][1],
			physicsTable["linearlock"][2],
		};
		data.angularLocks = {
			physicsTable["angularlock"][0],
			physicsTable["angularlock"][1],
			physicsTable["angularlock"][2],
		};

		RigidBodyComponent rigidBody{.bodyId = physics::createBody(data)};
		RigidBodyDataComponent rigidBodyData{.bodyData = data};

		return std::make_tuple(rigidBody, rigidBodyData);
	}

	void saveRigidBodyComponent(const std::tuple<RigidBodyComponent, RigidBodyDataComponent>& rigidBodyComponent,
								 nlohmann::json& physicsTable)
	{
		const auto& [rigidBody, rigidBodyData] = rigidBodyComponent;
		const auto& data = rigidBodyData.bodyData;

		physicsTable["mass"] = data.mass;

		switch (data.type)
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

		physicsTable["lineardamping"] = data.linearDamping;
		physicsTable["angulardamping"] = data.angularDamping;
		physicsTable["enablesleep"] = data.enableSleep;

		physicsTable["linearlock"] = {
			data.linearLocks[0],
			data.linearLocks[1],
			data.linearLocks[2],
		};
		physicsTable["angularlock"] = {
			data.angularLocks[0],
			data.angularLocks[1],
			data.angularLocks[2],
		};
	}

}