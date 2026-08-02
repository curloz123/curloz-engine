/**
 * @file rigidbodycomponent.cpp
 * @author curl0z
 * @brief displays rigid component's
 * data n all in inspector. Internally
 * can change rigid body's data and
 * body's attributes.
 */

#include "../../include/inspector/rigidbodycomponent.hpp"
#include "../../include/editor_types.hpp"
#include "../../include/offscreen/backend/descriptor.hpp"
#include "../../include/offscreen/backend/pipeline.hpp"
#include "../../include/offscreen/backend/ubo.hpp"
#include "../../include/scenetable.hpp"
#include "../../include/timemachine.hpp"
#include "core/logs.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "include/offscreen/offscreentarget.hpp"
#include "include/timemachine.hpp"
#include "math/angle.hpp"
#include "math/quateulerconv.hpp"
#include "math/worldtransform.hpp"
#include "physics/physicscomponent.hpp"
#include "physics/shape.hpp"
#include "renderer/model/model.hpp"
#include "renderer/pipelinedata/descriptor.hpp"
#include "renderer/rendercomponent.hpp"
#include "renderer/shapes.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/vk_types.hpp"
#include "window/inputmanager.hpp"
#include "window/mouse.hpp"
#include <imgui.h>

namespace clz::editor
{

void showBodySceneWindow();
void showBodyEditorWindow();

} // namespace clz::editor

namespace clz::editor
{
/**
 * @brief Shows rigid body header in
 * inspector Allows to edit physics-body
 * data of current selected entity Opens
 * a new window for shape editor
 * @note It currently in-complete. Needs
 * undo/redo work-done
 */
void showRigidBodyHeader()
{
	if (!ImGui::CollapsingHeader("RigidBody"))
		return;

	const auto& rigidBodyId =
		ecs::getComponent<physics::RigidBodyComponent>(currentSelectedEntity.value())
			.rigidBodyId;

	ImGui::PushFont(fontMonoBold);
	std::string type = "undefined";
	const auto& bodyType = physics::getBodyType(rigidBodyId);
	if (bodyType == physics::BodyType::DynamicBody)
		type = "dynamic";
	else if (bodyType == physics::BodyType::StaticBody)
		type = "static";
	else if (bodyType == physics::BodyType::KinematicBody)
		type = "kinematic";
	else
		clz::log::warn("In Editor, unable to identify body type");

	ImGui::Text("Body Type: ");
	ImGui::SameLine();
	ImGui::PopFont();
	ImGui::PushFont(fontMono);
	ImGui::Text(type.c_str());
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::PushFont(fontMonoBold);
	if (ImGui::Button("Change type"))
	{
		ImGui::OpenPopup("Body type");
		clz::log::debug("Changing body type");
	}
	ImGui::PopFont();
	if (ImGui::BeginPopup("Body type"))
	{
		const auto oldType = physics::getBodyType(rigidBodyId);
		if (ImGui::MenuItem("Static"))
		{
			timemachine::createSnapshot(
				[rigidBodyId, oldType] {
					physics::setBodyType(rigidBodyId, oldType);
				},
				[rigidBodyId] {
					physics::setBodyType(
						rigidBodyId,
						physics::BodyType::StaticBody
					);
				}
			);
			physics::setBodyType(rigidBodyId, physics::BodyType::StaticBody);
		}
		if (ImGui::MenuItem("Kinematic"))
		{
			timemachine::createSnapshot(
				[rigidBodyId, oldType] {
					physics::setBodyType(rigidBodyId, oldType);
				},
				[rigidBodyId] {
					physics::setBodyType(
						rigidBodyId,
						physics::BodyType::KinematicBody
					);
				}
			);
			physics::setBodyType(rigidBodyId, physics::BodyType::KinematicBody);
		}
		if (ImGui::MenuItem("Dynamic"))
		{
			timemachine::createSnapshot(
				[rigidBodyId, oldType] {
					physics::setBodyType(rigidBodyId, oldType);
				},
				[rigidBodyId] {
					physics::setBodyType(
						rigidBodyId,
						physics::BodyType::DynamicBody
					);
				}
			);
			physics::setBodyType(rigidBodyId, physics::BodyType::DynamicBody);
		}

		ImGui::EndPopup();
	}

	ImGui::PushFont(fontMono);
	ImGui::Separator();
	ImGui::Text("Computed Mass: %.2f", physics::getBodyMass(rigidBodyId));
	ImGui::Text(
		"Mass is computed "
		"via\ndensity of "
		"shapes attached "
		"to it"
	);
	ImGui::Separator();

	float linearDamping = physics::getBodyLinearDamping(rigidBodyId);
	ImGui::Text("Linear Damping");
	if (ImGui::InputFloat("##lineardamping: ", &linearDamping))
	{
		float oldLDamping = physics::getBodyLinearDamping(rigidBodyId);
		physics::setBodyLinearDamping(rigidBodyId, linearDamping);

		timemachine::createSnapshot(
			[rigidBodyId, oldLDamping] {
				physics::setBodyLinearDamping(rigidBodyId, oldLDamping);
			},
			[rigidBodyId, linearDamping] {
				physics::setBodyLinearDamping(rigidBodyId, linearDamping);
			}
		);
	}

	float angularDamping = physics::getBodyAngularDamping(rigidBodyId);
	ImGui::Text("Angular Damping");
	if (ImGui::InputFloat("##angulardamping: ", &angularDamping))
	{
		float oldADamping = physics::getBodyAngularDamping(rigidBodyId);
		physics::setBodyAngularDamping(rigidBodyId, angularDamping);

		timemachine::createSnapshot(
			[rigidBodyId, oldADamping] {
				physics::setBodyAngularDamping(rigidBodyId, oldADamping);
			},
			[rigidBodyId, angularDamping] {
				physics::setBodyAngularDamping(rigidBodyId, angularDamping);
			}
		);
	}

	bool sleepEnabled = physics::isSleepEnabled(rigidBodyId);
	ImGui::Text("Sleeping");
	if (ImGui::Checkbox("##sleeping", &sleepEnabled))
	{
		const auto oldSleep = physics::isSleepEnabled(rigidBodyId);
		physics::enableSleep(rigidBodyId, sleepEnabled);

		timemachine::createSnapshot(
			[rigidBodyId, oldSleep] {
				physics::enableSleep(rigidBodyId, oldSleep);
			},
			[rigidBodyId, sleepEnabled] {
				physics::enableSleep(rigidBodyId, sleepEnabled);
			}
		);
	}

	auto linearLocks = physics::getBodyLinearLocks(rigidBodyId);
	ImGui::Text("Linear lock");
	if (ImGui::Checkbox("X##xlinear", &linearLocks[0]))
	{
		const auto oldLinearLocks = physics::getBodyLinearLocks(rigidBodyId);
		physics::setBodyLinearLocks(rigidBodyId, linearLocks);

		timemachine::createSnapshot(
			[rigidBodyId, oldLinearLocks] {
				physics::setBodyLinearLocks(rigidBodyId, oldLinearLocks);
			},
			[rigidBodyId, linearLocks] {
				physics::setBodyLinearLocks(rigidBodyId, linearLocks);
			}
		);
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Y##ylinear", &linearLocks[1]))
	{
		const auto oldLinearLocks = physics::getBodyLinearLocks(rigidBodyId);
		physics::setBodyLinearLocks(rigidBodyId, linearLocks);

		timemachine::createSnapshot(
			[rigidBodyId, oldLinearLocks] {
				physics::setBodyLinearLocks(rigidBodyId, oldLinearLocks);
			},
			[rigidBodyId, linearLocks] {
				physics::setBodyLinearLocks(rigidBodyId, linearLocks);
			}
		);
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Z##zlinear", &linearLocks[2]))
	{
		const auto oldLinearLocks = physics::getBodyLinearLocks(rigidBodyId);
		physics::setBodyLinearLocks(rigidBodyId, linearLocks);

		timemachine::createSnapshot(
			[rigidBodyId, oldLinearLocks] {
				physics::setBodyLinearLocks(rigidBodyId, oldLinearLocks);
			},
			[rigidBodyId, linearLocks] {
				physics::setBodyLinearLocks(rigidBodyId, linearLocks);
			}
		);
	}

	auto angularLocks = physics::getBodyAngularLocks(rigidBodyId);
	ImGui::Text("Angular lock");
	if (ImGui::Checkbox("X##xangular", &angularLocks[0]))
	{
		const auto oldAngularLocks = physics::getBodyAngularLocks(rigidBodyId);
		physics::setBodyAngularLocks(rigidBodyId, angularLocks);

		timemachine::createSnapshot(
			[rigidBodyId, oldAngularLocks] {
				physics::setBodyAngularLocks(rigidBodyId, oldAngularLocks);
			},
			[rigidBodyId, angularLocks] {
				physics::setBodyAngularLocks(rigidBodyId, angularLocks);
			}
		);
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Y##yangular", &angularLocks[1]))
	{
		const auto oldAngularLocks = physics::getBodyAngularLocks(rigidBodyId);
		physics::setBodyAngularLocks(rigidBodyId, angularLocks);

		timemachine::createSnapshot(
			[rigidBodyId, oldAngularLocks] {
				physics::setBodyAngularLocks(rigidBodyId, oldAngularLocks);
			},
			[rigidBodyId, angularLocks] {
				physics::setBodyAngularLocks(rigidBodyId, angularLocks);
			}
		);
	}

	ImGui::SameLine();
	if (ImGui::Checkbox("Z##zangular", &angularLocks[2]))
	{
		const auto oldAngularLocks = physics::getBodyAngularLocks(rigidBodyId);
		physics::setBodyAngularLocks(rigidBodyId, angularLocks);

		timemachine::createSnapshot(
			[rigidBodyId, oldAngularLocks] {
				physics::setBodyAngularLocks(rigidBodyId, oldAngularLocks);
			},
			[rigidBodyId, angularLocks] {
				physics::setBodyAngularLocks(rigidBodyId, angularLocks);
			}
		);
	}

	if ((ImGui::Button("Add Shape") && !physicsBodyShapeImage.showTarget))
	{
		renderer::resetCamera(physicsBodyShapeImage.cameraId);
		physicsBodyShapeImage.showTarget = true;

		capturedBodyData = {
			.entityId = currentSelectedEntity.value(),
			.type = physics::getBodyType(rigidBodyId),
			.enableSleep = physics::isSleepEnabled(rigidBodyId),
			.position = physics::getBodyPosition(rigidBodyId),
			.rotation = physics::getBodyRotation(rigidBodyId),
			.linearDamping = physics::getBodyLinearDamping(rigidBodyId),
			.angularDamping = physics::getBodyAngularDamping(rigidBodyId),
			.linearLocks = physics::getBodyLinearLocks(rigidBodyId),
			.angularLocks = physics::getBodyAngularLocks(rigidBodyId),
			.shapeData = [rigidBodyId] {
				std::vector<physics::ShapeDef> shapeData;
				const auto& shapes = physics::getBodyShapes(rigidBodyId);
				shapeData.reserve(shapes.size());
				for (size_t i = 0; i < shapes.size(); ++i)
				{
					shapeData.emplace_back(shapes[i].getShapeData());
				}
				return shapeData;
			}()
		};
	}

	ImGui::PopFont();
}

void showBodyEditorWindow()
{
	if (!capturedBodyData.entityId.has_value())
		return;

	if (!ImGui::Begin("Shape Controls"))
	{
		ImGui::End();
		return;
	}

	ImGui::PushFont(fontMono);
	ImGui::Text(("Currently editing: "));
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::PushFont(fontMonoBold);
	ImGui::Text(ecs::getEntityName(capturedBodyData.entityId.value()).c_str());

	ImGui::PopFont();
	ImGui::Separator();

	const auto rigidBodyId = ecs::getComponent<physics::RigidBodyComponent>(
					 capturedBodyData.entityId.value()
	)
					 .rigidBodyId;

	const char* shapeType = "Shape type";
	if (ImGui::Button("Add Shape"))
	{
		ImGui::OpenPopup(shapeType);
		clz::log::debug("Adding shape");
	}
	if (ImGui::BeginPopup(shapeType))
	{
		if (ImGui::MenuItem("Box"))
		{
			physics::ShapeDef shapeDef(physics::ShapeType::BOX);
			const auto newShapeIndex =
				physics::getBodyShapes(rigidBodyId).size();
			physics::attachShapeToBody(rigidBodyId, shapeDef);
			capturedBodyData.shapeData.emplace_back(shapeDef);

			timemachine::createSnapshot(
				[rigidBodyId, newShapeIndex] {
					auto& shapes = physics::getBodyShapes(rigidBodyId);
					shapes[newShapeIndex].destroyShape();
					capturedBodyData.shapeData[newShapeIndex]
						.shouldBeDestroyed = true;
				},
				[rigidBodyId, newShapeIndex] {
					auto& shapes = physics::getBodyShapes(rigidBodyId);
					shapes[newShapeIndex].createShape(
						capturedBodyData.shapeData[newShapeIndex],
						rigidBodyId
					);
					capturedBodyData.shapeData[newShapeIndex]
						.shouldBeDestroyed = false;
				}
			);

			clz::log::debug(
				"Added box shape, "
				"click save to apply changes"
			);
		}
		ImGui::EndPopup();
	}

	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		physics::refreshAttachedShapes(rigidBodyId);
		timemachine::clearSnapshots();

		capturedBodyData.shapeData = [rigidBodyId] {
			std::vector<physics::ShapeDef> shapeData;
			const auto& shapes = physics::getBodyShapes(rigidBodyId);
			shapeData.reserve(shapes.size());
			for (size_t i = 0; i < shapes.size(); ++i)
			{
				shapeData.emplace_back(shapes[i].getShapeData());
			}
			return shapeData;
		}();

		clz::log::debug("Saved changes");
	}

	ImGui::SameLine();
	const char* closeWindow = "Close";
	if (ImGui::Button(closeWindow))
	{
		ImGui::OpenPopup(closeWindow);
	}
	if (ImGui::BeginPopup(closeWindow))
	{
		if (ImGui::MenuItem("Sure"))
		{
			physics::refreshAttachedShapes(rigidBodyId);
			physicsBodyShapeImage.showTarget = false;
			renderer::resetCamera(physicsBodyShapeImage.cameraId);
			timemachine::clearSnapshots();
			ImGui::EndPopup();
			ImGui::End();
			clz::log::debug("Exiting");
			return;
		}
		if (ImGui::MenuItem("Not yet"))
		{
			clz::log::debug("Not exiting");
			//
		}
		ImGui::EndPopup();
	}
	ImGui::Separator();

	ImGui::PushFont(fontMonoBold);
	ImGui::Text("Attached shapes");
	ImGui::PopFont();

	auto& shapes = physics::getBodyShapes(rigidBodyId);
	CLZ_ASSERT(
		capturedBodyData.shapeData.size() == shapes.size(),
		"wtf have you done with shape data"
	);

	for (size_t i = 0; i < capturedBodyData.shapeData.size(); ++i)
	{
		auto& shape = shapes[i];
		auto& shapeData = capturedBodyData.shapeData[i];
		if (shape.isItTimeSon())
			continue;

		ImGui::PushFont(fontSans);
		std::string shapeName = "Shape " + std::to_string(i);

		ImGui::PushID(shapeName.c_str());
		ImGui::Button(shapeName.c_str());
		ImGui::PopFont();

		if (ImGui::InputFloat("Density", &shapeData.density))
		{
			const auto oldDensity = shape.getDensity();
			const auto newDensity = shapeData.density;
			shape.setDensity(shapeData.density);

			timemachine::createSnapshot(
				[rigidBodyId, i, oldDensity] {
					if (ecs::getComponent<physics::RigidBodyComponent>(
						    capturedBodyData.entityId.value()
					    )
						    .rigidBodyId != rigidBodyId)
					{
						return;
					}
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					auto& shapeData = capturedBodyData.shapeData[i];
					shape.setDensity(oldDensity);
					shapeData.density = oldDensity;
				},
				[rigidBodyId, i, newDensity] {
					if (ecs::getComponent<physics::RigidBodyComponent>(
						    capturedBodyData.entityId.value()
					    )
						    .rigidBodyId != rigidBodyId)
					{
						return;
					}
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					auto& shapeData = capturedBodyData.shapeData[i];
					shape.setDensity(newDensity);
					shapeData.density = newDensity;
				}
			);
		}

		if (ImGui::InputFloat("Friction", &shapeData.friction))
		{
			const auto oldFriction = shape.getFriction();
			const auto newFriction = shapeData.friction;
			shape.setFriction(shapeData.friction);

			timemachine::createSnapshot(
				[rigidBodyId, i, oldFriction] {
					if (ecs::getComponent<physics::RigidBodyComponent>(
						    capturedBodyData.entityId.value()
					    )
						    .rigidBodyId != rigidBodyId)
					{
						return;
					}
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					auto& shapeData = capturedBodyData.shapeData[i];
					shape.setFriction(oldFriction);
					shapeData.friction = oldFriction;
				},
				[rigidBodyId, i, newFriction] {
					if (ecs::getComponent<physics::RigidBodyComponent>(
						    capturedBodyData.entityId.value()
					    )
						    .rigidBodyId != rigidBodyId)
					{
						return;
					}
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					auto& shapeData = capturedBodyData.shapeData[i];
					shape.setFriction(newFriction);
					shapeData.friction = newFriction;
				}
			);
		}

		if (ImGui::InputFloat("Restitution", &shapeData.restitution))
		{
			const auto oldRestitution = shape.getRestitution();
			const auto newRestitution = shapeData.restitution;
			shape.setRestitution(newRestitution);
			timemachine::createSnapshot(
				[rigidBodyId, i, oldRestitution] {
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					capturedBodyData.shapeData[i].restitution =
						oldRestitution;
					shape.setRestitution(oldRestitution);
				},
				[rigidBodyId, i, newRestitution] {
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					capturedBodyData.shapeData[i].restitution =
						newRestitution;
					shape.setRestitution(newRestitution);
				}
			);
		}

		ImGui::InputFloat3("Local position", &shapeData.position.x);
		if (ImGui::IsItemDeactivated())
		{
			shapeData.position.x =
				std::clamp(shapeData.position.x, -30.0f, 30.0f);
			shapeData.position.y =
				std::clamp(shapeData.position.y, -30.0f, 30.0f);
			shapeData.position.z =
				std::clamp(shapeData.position.z, -30.0f, 30.0f);

			const auto oldPos = shape.getPosition();
			const auto newPos = shapeData.position;
			shape.setPosition(newPos);

			timemachine::createSnapshot(
				[rigidBodyId, i, oldPos] {
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					capturedBodyData.shapeData[i].position = oldPos;
					shape.setPosition(oldPos);
				},
				[rigidBodyId, i, newPos] {
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					capturedBodyData.shapeData[i].position = newPos;
					shape.setPosition(newPos);
				}
			);
		}

		if (ImGui::InputFloat3("Local rotation", &shapeData.rotation.x))
		{
			shapeData.rotation.x =
				std::clamp(shapeData.rotation.x, -179.0f, 179.0f);
			shapeData.rotation.y =
				std::clamp(shapeData.rotation.y, -179.0f, 179.0f);
			shapeData.rotation.z =
				std::clamp(shapeData.rotation.z, -179.0f, 179.0f);

			const auto oldRot = shape.getRotation();
			const auto newRot = shapeData.rotation;
			shape.setRotation(newRot);

			timemachine::createSnapshot(
				[rigidBodyId, i, oldRot] {
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					capturedBodyData.shapeData[i].rotation = oldRot;
					shape.setRotation(oldRot);
				},
				[rigidBodyId, i, newRot] {
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					capturedBodyData.shapeData[i].rotation = newRot;
					shape.setRotation(newRot);
				}
			);
		}

		auto showLBH = [](physics::RigidBodyId rigidBodyId,
				  const int index,
				  const physics::ShapeType type) -> void {
			auto& shape = physics::getBodyShapes(rigidBodyId)[index];
			auto& shapeData = capturedBodyData.shapeData[index];
			switch (type)
			{
			case (physics::ShapeType::BOX): {
				if (ImGui::InputFloat3(
					    "Half dimensions",
					    &shapeData.halfExtents.x
				    ))
				{
					shapeData.halfExtents.x = std::clamp(
						shapeData.halfExtents.x,
						0.1f,
						30.0f
					);

					shapeData.halfExtents.y = std::clamp(
						shapeData.halfExtents.y,
						0.1f,
						30.0f
					);

					shapeData.halfExtents.z = std::clamp(
						shapeData.halfExtents.z,
						0.1f,
						30.0f
					);

					const auto oldExtents = shape.getBoxHalfExtents();
					const auto newExtents = shapeData.halfExtents;
					shape.setBoxHalfExtents(newExtents);

					timemachine::createSnapshot(
						[rigidBodyId, index, oldExtents] {
							auto& shape = physics::getBodyShapes(
								rigidBodyId
							)[index];
							capturedBodyData.shapeData[index]
								.halfExtents = oldExtents;
							shape.setBoxHalfExtents(oldExtents);
						},
						[rigidBodyId, index, newExtents] {
							auto& shape = physics::getBodyShapes(
								rigidBodyId
							)[index];
							capturedBodyData.shapeData[index]
								.halfExtents = newExtents;
							shape.setBoxHalfExtents(newExtents);
						}
					);
				}
				break;
			}

			case (physics::ShapeType::CAPSULE):
			case (physics::ShapeType::CYLINDER): {
				if (ImGui::InputFloat("Height", &shapeData.height))
				{
					shapeData.height =
						std::clamp(shapeData.height, 0.1f, 30.0f);

					shape.setCapsuleHeight(shapeData.height);
				}
				break;
			}

			default: {
				CLZ_ASSERT(
					false,
					"Unknown shape type passed while "
					"requesting LBH"
				);
			}
			}
		};

		auto showRadius = [](physics::Shape& shape,
				     physics::ShapeDef& shapeData,
				     const physics::ShapeType type) -> void {
			switch (type)
			{
			case (physics::ShapeType::SPHERE): {
				if (ImGui::InputFloat("Radius", &shapeData.radius))
				{
					shapeData.radius =
						std::clamp(shapeData.radius, 0.1f, 30.0f);

					shape.setSphereRadius(shapeData.radius);
				}
			}
			case (physics::ShapeType::CAPSULE):
			case (physics::ShapeType::CYLINDER): {
				auto radius = shape.getCapsuleRadius();
				ImGui::SliderFloat("Radius", &radius, 0.1f, 30.0f, "%.2f");
				if (ImGui::IsItemDeactivated())
				{
					shape.setCapsuleRadius(radius);
				}

				break;
			}

			default: {

				CLZ_ASSERT(
					false,
					"Unknown shape type passed "
					"while requesting LBH"
				);
			}
			}
		};

		switch (shape.getShapeType())
		{
		case (physics::ShapeType::BOX): {
			ImGui::PushFont(fontMono);
			ImGui::Text("Shape type: ");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushFont(fontMonoBold);
			ImGui::Text("Box");
			ImGui::PopFont();

			showLBH(rigidBodyId, i, physics::ShapeType::BOX);

			break;
		}
		case (physics::ShapeType::SPHERE): {
			ImGui::PushFont(fontMono);
			ImGui::Text(
				"Shape "
				"type:"
				" "
			);
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushFont(fontMonoBold);
			ImGui::Text("Box");
			ImGui::PopFont();

			showRadius(shape, shapeData, physics::ShapeType::SPHERE);

			break;
		}
		case (physics::ShapeType::CAPSULE): {
			ImGui::PushFont(fontMono);
			ImGui::Text("Shape type: ");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushFont(fontMonoBold);
			ImGui::Text("Capsule");
			ImGui::PopFont();

			showLBH(rigidBodyId, i, physics::ShapeType::CAPSULE);
			showRadius(shape, shapeData, physics::ShapeType::CAPSULE);
		}
		case (physics::ShapeType::CYLINDER): {
			ImGui::PushFont(fontMono);
			ImGui::Text("Shape type: ");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushFont(fontMonoBold);
			ImGui::Text("Capsule");
			ImGui::PopFont();

			showLBH(rigidBodyId, i, physics::ShapeType::CYLINDER);
			showRadius(shape, shapeData, physics::ShapeType::CYLINDER);
		}
		}

		if (ImGui::Button("Delete Shape"))
		{
			shape.destroyShape();
			shapeData.shouldBeDestroyed = true;

			timemachine::createSnapshot(
				[rigidBodyId, i] {
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					shape.createShape(
						capturedBodyData.shapeData[i],
						rigidBodyId
					);
					capturedBodyData.shapeData[i].shouldBeDestroyed =
						false;
				},
				[rigidBodyId, i] {
					auto& shape = physics::getBodyShapes(rigidBodyId)[i];
					shape.destroyShape();
					capturedBodyData.shapeData[i].shouldBeDestroyed =
						true;
				}
			);
		}

		ImGui::Separator();
		ImGui::PopID();
	}

	ImGui::End();
}

void showBodySceneWindow()
{
	if (!capturedBodyData.entityId.has_value())
		return;

	ImGui::Begin(
		"RigidBody Shape "
		"Editor"
	);

	const ImVec2 avail = ImGui::GetContentRegionAvail();
	if (avail.x < 1.0f || avail.y < 1.0f)
	{
		ImGui::End();
		return;
	}

	const auto width = static_cast<uint32_t>(avail.x);
	const auto height = static_cast<uint32_t>(avail.y);

	if ((width != physicsBodyShapeImage.extent.width ||
	     height != physicsBodyShapeImage.extent.height) &&
	    clz::window::isMouseReleased(clz::input::Mouse::MouseLeft))
	{
		physicsBodyShapeImage.extent.width = width;
		physicsBodyShapeImage.extent.height = height;
		renderer::updateCameraProjMatrix(physicsBodyShapeImage.cameraId);
		physicsBodyShapeImage.outDated = true;
		clz::log::debug(
			"Won't show "
			"image this "
			"frame, "
			"resizing rn"
		);
		ImGui::End();
		return;
	}

	if (ImGui::IsWindowFocused())
	{
		static bool rightClickThisFrame = false;
		static bool rightClickLastFrame = false;
		if (window::isMousePressed(clz::input::Mouse::MouseRight))
		{
			rightClickThisFrame = true;
		}
		else
		{
			rightClickThisFrame = false;
		}

		if (rightClickThisFrame)
		{
			const auto Id = physicsBodyShapeImage.cameraId;
			renderer::useCamera(Id);
			renderer::updateCamera(Id);
		}

		if (rightClickThisFrame && !rightClickLastFrame)
		{
			window::disableCursor();
		}
		else if (!rightClickThisFrame && rightClickLastFrame)
		{
			window::enableCursor();
			renderer::setCameraFirstTime(physicsBodyShapeImage.cameraId);
		}

		rightClickLastFrame = rightClickThisFrame;
	}

	const ImVec2 cursorPosBefore = ImGui::GetCursorScreenPos();

	ImGui::Image((ImTextureID)physicsBodyShapeImage.descriptorSet, avail);

	ImGui::End();
}

void presentBodyEditorWindow()
{
	showBodyEditorWindow();
	showBodySceneWindow();
}

void drawBodyEditorOffscreenImage(VkCommandBuffer commandBuffer)
{
	if (!capturedBodyData.entityId.has_value())
	{
		return;
	}
	prepareOffscreenTarget(physicsBodyShapeImage);
	renderer::transition_image_layout(
		physicsBodyShapeImage.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		0,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
		VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		VK_IMAGE_ASPECT_COLOR_BIT,
		commandBuffer
	);

	VkRenderingAttachmentInfoKHR colorAttachment = {};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	colorAttachment.pNext = nullptr;
	colorAttachment.imageView = physicsBodyShapeImage.imageView;
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue = {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}};

	VkRenderingAttachmentInfoKHR depthAttachment = {};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	depthAttachment.imageView = physicsBodyShapeImage.depthImageView;
	depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.clearValue.depthStencil.depth = 1.0f;

	VkRenderingInfoKHR renderingInfo = {};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
	renderingInfo.pNext = nullptr, renderingInfo.flags = 0,
	renderingInfo.renderArea = {{0, 0}, physicsBodyShapeImage.extent},
	renderingInfo.layerCount = 1, renderingInfo.colorAttachmentCount = 1,
	renderingInfo.pColorAttachments = &colorAttachment,
	renderingInfo.pDepthAttachment = &depthAttachment;

	vkCmdBeginRendering(commandBuffer, &renderingInfo);

	const VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(physicsBodyShapeImage.extent.width),
		.height = static_cast<float>(physicsBodyShapeImage.extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	const VkRect2D scissor{{0, 0}, physicsBodyShapeImage.extent};

	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		backend::editorPipelineContext.pipeline
	);
	// Update uniform buffers here

	const auto cameraId = physicsBodyShapeImage.cameraId;
	const auto p = math::makePerspectiveMatrix(
		renderer::getCameraFarPlane(cameraId),
		renderer::getCameraNearPlane(cameraId),
		viewport.width / viewport.height,
		math::radians(renderer::getCameraFov(cameraId))
	);
	const auto v = math::makeViewMatrix(
		renderer::getCameraPosition(cameraId),
		renderer::getCameraTarget(cameraId),
		renderer::WorldUp
	);
	const auto ubo = backend::CameraShaderUBO{.projection = p, .view = v};
	memcpy(backend::editorCameraUBO.mapped[renderer::r_currentFrame], &ubo, sizeof(ubo));

	const std::array descriptorSets = {
		backend::cameraDescriptorSets[renderer::r_currentFrame],
		renderer::samplerDescriptorSets[renderer::r_currentFrame]
	};
	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		backend::editorPipelineContext.layout,
		0,
		descriptorSets.size(),
		descriptorSets.data(),
		0,
		nullptr
	);

	renderer::drawModel(
		ecs::getComponent<renderer::ModelComponent>(
			capturedBodyData.entityId.value()
		)
			.modelId,
		math::vec3(0.0f),
		math::quat(1.0f, 0.0f, 0.0f, 0.0f),
		ecs::getComponent<ecs::TransformComponent>(capturedBodyData.entityId.value())
			.scale,
		commandBuffer
	);

	const auto& shapes = physics::getBodyShapes(
		ecs::getComponent<physics::RigidBodyComponent>(
			capturedBodyData.entityId.value()
		)
			.rigidBodyId
	);

	for (const auto& shapeData : capturedBodyData.shapeData)
	{
		if (shapeData.shouldBeDestroyed)
			continue;

		const auto quat = math::quatFromEuler(shapeData.rotation);
		const auto pos = shapeData.position;
		math::vec3 scale;
		renderer::Shape drawShape;
		switch (shapeData.shapeType)
		{
		case physics::ShapeType::BOX: {
			scale = shapeData.halfExtents * 2;
			drawShape = renderer::Shape::BOX;
			break;
		}

		case physics::ShapeType::SPHERE:
			break;
		case physics::ShapeType::CAPSULE:
			break;
		case physics::ShapeType::CYLINDER:
			break;

		default:
			/// handle this case
		}
		renderer::drawShape(
			commandBuffer,
			drawShape,
			p,
			v,
			math::getModelMatrix(quat, pos, scale),
			math::vec4(0.0f, 0.5f, 0.0f, 1.0f)
		);
	}

	vkCmdEndRendering(commandBuffer);
	renderer::transition_image_layout(
		physicsBodyShapeImage.image,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
		0,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR,
		VK_IMAGE_ASPECT_COLOR_BIT,
		commandBuffer
	);
}
} // namespace clz::editor
