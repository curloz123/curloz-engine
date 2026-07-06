/**
 * @file rigidbodycomponent.cpp
 * @author curl0z
 * @brief displays rigid component's data n all
 * in inspector. Internally can change rigid body's
 * data and body's attributes.
 */


#include "../../include/inspector/rigidbodycomponent.hpp"
#include "../../include/editor_types.hpp"
#include "../../include/timemachine.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"
#include "imgui.h"
#include "core/logs.hpp"

namespace clz::editor
{
	/// @brief Stores previous data in rigid body data component
	ecs::RigidBodyDataComponent previousRigidBodyData;
	/// @brief Stores previous current in rigid body data component
	ecs::RigidBodyDataComponent currentRigidBodyData;

	/// @brief Shows rigid body header in inspector
	/// also stores a snapshot upon every change
	void showRigidBodyHeader()
	{
		if (!ImGui::CollapsingHeader("RigidBody"))
			return;

		const auto& body = ecs::getComponent<ecs::RigidBodyComponent>(currentSelectedEntity.value());
		const auto& data = ecs::getComponent<ecs::RigidBodyDataComponent>(currentSelectedEntity.value());
		currentRigidBodyData = data;


		ImGui::PushFont(fontMonoBold);
		std::string type = "undefined";
		if (data.bodyData.type == physics::BodyType::DynamicBody)
			type = "dynamic";
		else if (data.bodyData.type == physics::BodyType::StaticBody)
			type = "static";
		else if (data.bodyData.type == physics::BodyType::KinematicBody)
			type = "kinematic";
		else
			clz::log::warn("In Editor, unable to identify body type");
		ImGui::Text("Body Type: %s", type.c_str());
		ImGui::PopFont();

		bool anyEditFinished = false;
		bool enterCommit = false;
		bool defocusCommit = false;

		ImGui::PushFont(fontMono);
		ImGui::Text("Mass");
		enterCommit = ImGui::InputFloat("##mass",
			&currentRigidBodyData.bodyData.mass, 0.01f, 1000.0f,
			"%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
		defocusCommit = ImGui::IsItemDeactivatedAfterEdit();
		if (enterCommit || defocusCommit)
		{
			physics::setMass(body.bodyId, currentRigidBodyData.bodyData.mass);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}

		ImGui::Text("Linear Damping");
		enterCommit = ImGui::InputFloat("##lineardamping: ",
			&currentRigidBodyData.bodyData.linearDamping, 0.0f, 1.0f,
			"%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
		defocusCommit = ImGui::IsItemDeactivatedAfterEdit();
		if (enterCommit || defocusCommit)
		{
			physics::setBodyLinearDamping(body.bodyId, currentRigidBodyData.bodyData.linearDamping);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}

		ImGui::Text("Angular Damping");
		enterCommit = ImGui::InputFloat("##angulardamping: ",
			&currentRigidBodyData.bodyData.linearDamping, 0.0f, 1.0f,
			"%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
		defocusCommit = ImGui::IsItemDeactivatedAfterEdit();
		if (enterCommit || defocusCommit)
		{
			physics::setBodyAngularDamping(body.bodyId, currentRigidBodyData.bodyData.angularDamping);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}

		ImGui::Text("Sleeping");
		if (ImGui::Checkbox("##sleeping", &currentRigidBodyData.bodyData.enableSleep))
		{
			physics::enableSleep(body.bodyId, currentRigidBodyData.bodyData.enableSleep);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}

		ImGui::Text("Linear lock");
		if (ImGui::Checkbox("X##xlinear", &currentRigidBodyData.bodyData.linearLocks[0]))
		{
			physics::setLinearLock(body.bodyId, currentRigidBodyData.bodyData.linearLocks);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Y##ylinear", &currentRigidBodyData.bodyData.linearLocks[1]))
		{
			physics::setLinearLock(body.bodyId, currentRigidBodyData.bodyData.linearLocks);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Z##zlinear", &currentRigidBodyData.bodyData.linearLocks[2]))
		{
			physics::setLinearLock(body.bodyId, currentRigidBodyData.bodyData.linearLocks);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}
		ImGui::Text("Angular lock");
		if (ImGui::Checkbox("X##xangular", &currentRigidBodyData.bodyData.angularLocks[0]))
		{
			physics::setAngularLock(body.bodyId, currentRigidBodyData.bodyData.angularLocks);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Y##yangular", &currentRigidBodyData.bodyData.angularLocks[1]))
		{
			physics::setAngularLock(body.bodyId, currentRigidBodyData.bodyData.angularLocks);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Z##zangular", &currentRigidBodyData.bodyData.angularLocks[2]))
		{
			physics::setAngularLock(body.bodyId, currentRigidBodyData.bodyData.angularLocks);
			previousRigidBodyData = data;
			anyEditFinished = true;
		}
		ImGui::PopFont();


		if (anyEditFinished)
		{
			ecs::setComponent<ecs::RigidBodyDataComponent>(currentSelectedEntity.value(), currentRigidBodyData);

			clz::log::debug("Creating a snapshot of rigidbody component data");
			createSnapshot<ecs::RigidBodyDataComponent>(currentSelectedEntity.value(), previousRigidBodyData, currentRigidBodyData);
		}
	}
} // namespace clz::editor