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
#include "physics/body.hpp"
#include "physics/physics.hpp"
#include "physics/physicscomponent.hpp"
#include "physics/shape.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/model/model.hpp"
#include "renderer/pipelinedata/descriptor.hpp"
#include "renderer/rendercomponent.hpp"
#include "renderer/shapes.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/vk_types.hpp"
#include "window/inputmanager.hpp"
#include "window/mouse.hpp"
#include <imgui.h>
#include "../../include/gizmo/gizmo.hpp"

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
	 * @note Is currently in-complete. Needs
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
			auto createBodyTypeSnapshot = [rigidBodyId](
					physics::BodyType oldType, 
					physics::BodyType newType)
			{
				timemachine::createSnapshot(
					[rigidBodyId, oldType] {
						physics::setBodyType(rigidBodyId, oldType);
					},
					[rigidBodyId, newType] {
						physics::setBodyType(rigidBodyId, newType);
					}
				);
				
			};
			if (ImGui::MenuItem("Static"))
			{
				createBodyTypeSnapshot(oldType, physics::BodyType::StaticBody);
				physics::setBodyType(rigidBodyId, physics::BodyType::StaticBody);
			}
			if (ImGui::MenuItem("Kinematic"))
			{
				createBodyTypeSnapshot(oldType, physics::BodyType::KinematicBody);
				physics::setBodyType(rigidBodyId, physics::BodyType::KinematicBody);
			}
			if (ImGui::MenuItem("Dynamic"))
			{
				createBodyTypeSnapshot(oldType, physics::BodyType::DynamicBody);
				physics::setBodyType(rigidBodyId, physics::BodyType::DynamicBody);
			}

			ImGui::EndPopup();
		}

		ImGui::PushFont(fontMono);
		ImGui::Separator();
		ImGui::Text("Computed Mass: %.2f", physics::getBodyMass(rigidBodyId));
		ImGui::Text(
			"Mass is computed "
			"via density of "
			"shapes attached "
			"to it"
		);
		ImGui::Separator();

		auto createAttributeSnapshot = [rigidBodyId](
				const auto func, 
				const auto oldValue, 
				const auto newValue)
		{
			timemachine::createSnapshot(
				[func, rigidBodyId, oldValue]{
					func(rigidBodyId, oldValue);
				},
				[func, rigidBodyId, newValue]{
					func(rigidBodyId, newValue);
				}
			);
		};

		float linearDamping = physics::getBodyLinearDamping(rigidBodyId);
		ImGui::Text("Linear Damping");

		if (ImGui::InputFloat("##lineardamping: ", &linearDamping))
		{
			const float oldLDamping = physics::getBodyLinearDamping(rigidBodyId);
			const float newLDamping = linearDamping;

			physics::setBodyLinearDamping(rigidBodyId, linearDamping);
			createAttributeSnapshot(physics::setBodyLinearDamping, oldLDamping, newLDamping);
		}

		float angularDamping = physics::getBodyAngularDamping(rigidBodyId);
		ImGui::Text("Angular Damping");
		if (ImGui::InputFloat("##angulardamping: ", &angularDamping))
		{
			float oldADamping = physics::getBodyAngularDamping(rigidBodyId);
			physics::setBodyAngularDamping(rigidBodyId, angularDamping);

			createAttributeSnapshot(physics::setBodyAngularDamping, oldADamping, angularDamping);
		}

		bool sleepEnabled = physics::isSleepEnabled(rigidBodyId);
		ImGui::Text("Sleeping");
		if (ImGui::Checkbox("##sleeping", &sleepEnabled))
		{
			const auto oldSleep = physics::isSleepEnabled(rigidBodyId);
			physics::enableSleep(rigidBodyId, sleepEnabled);

			createAttributeSnapshot(physics::enableSleep, oldSleep, sleepEnabled);
		}

		auto linearLocks = physics::getBodyLinearLocks(rigidBodyId);
		ImGui::Text("Linear lock");
		if (ImGui::Checkbox("X##xlinear", &linearLocks[0]))
		{
			const auto oldLinearLocks = physics::getBodyLinearLocks(rigidBodyId);
			physics::setBodyLinearLocks(rigidBodyId, linearLocks);

			createAttributeSnapshot(physics::setBodyLinearLocks, oldLinearLocks, linearLocks);
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Y##ylinear", &linearLocks[1]))
		{
			const auto oldLinearLocks = physics::getBodyLinearLocks(rigidBodyId);
			physics::setBodyLinearLocks(rigidBodyId, linearLocks);

			createAttributeSnapshot(physics::setBodyLinearLocks, oldLinearLocks, linearLocks);
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Z##zlinear", &linearLocks[2]))
		{
			const auto oldLinearLocks = physics::getBodyLinearLocks(rigidBodyId);
			physics::setBodyLinearLocks(rigidBodyId, linearLocks);

			createAttributeSnapshot(physics::setBodyLinearLocks, oldLinearLocks, linearLocks);
		}

		auto angularLocks = physics::getBodyAngularLocks(rigidBodyId);
		ImGui::Text("Angular lock");
		if (ImGui::Checkbox("X##xangular", &angularLocks[0]))
		{
			const auto oldAngularLocks = physics::getBodyAngularLocks(rigidBodyId);
			physics::setBodyAngularLocks(rigidBodyId, angularLocks);

			createAttributeSnapshot(physics::setBodyAngularLocks, oldAngularLocks, angularLocks);
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Y##yangular", &angularLocks[1]))
		{
			const auto oldAngularLocks = physics::getBodyAngularLocks(rigidBodyId);
			physics::setBodyAngularLocks(rigidBodyId, angularLocks);

			createAttributeSnapshot(physics::setBodyAngularLocks, oldAngularLocks, angularLocks);
		}

		ImGui::SameLine();
		if (ImGui::Checkbox("Z##zangular", &angularLocks[2]))
		{
			const auto oldAngularLocks = physics::getBodyAngularLocks(rigidBodyId);
			physics::setBodyAngularLocks(rigidBodyId, angularLocks);

			createAttributeSnapshot(physics::setBodyAngularLocks, oldAngularLocks, angularLocks);
		}

		if ((ImGui::Button("Edit Shapes") && !physicsBodyShapeImage.showTarget))
		{
			renderer::resetCamera(physicsBodyShapeImage.cameraId);
			capturedEntity = currentSelectedEntity.value();
			physicsBodyShapeImage.showTarget = true;
		}

		ImGui::PopFont();
	}

	void presentBodyEditorWindow()
	{
		showBodyEditorWindow();
		showBodySceneWindow();
	}

	void showBodyEditorWindow()
	{
		if (!capturedEntity.has_value())
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
		ImGui::Text(ecs::getEntityName(capturedEntity.value()).c_str());

		ImGui::PopFont();
		ImGui::Separator();

		const auto rigidBodyId = ecs::getComponent<physics::RigidBodyComponent>(
						 capturedEntity.value()).rigidBodyId;

		const char* shapeType = "Shape type";
		if (ImGui::Button("Add Shape"))
		{
			ImGui::OpenPopup(shapeType);
			clz::log::debug("Adding shape");
		}
		if (ImGui::BeginPopup(shapeType))
		{
			auto createShapeAddSnapshot = [](physics::Shape& shape)
			{
				timemachine::createSnapshot(
					[&shape] {
						shape.markForDeletion();
					},
					[&shape] {
						shape.unMarkForDeletion();
					}
				);
			};

			if (ImGui::MenuItem("Box"))
			{
				physics::ShapeDef shapeDef(physics::ShapeType::BOX);
				auto shapeId = physics::attachShapeToBody(rigidBodyId, shapeDef);
				auto& shape = physics::getBodyShape(rigidBodyId, shapeId);
				lastEditedShape = shapeId;
				createShapeAddSnapshot(shape);
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
				capturedEntity.reset();
				lastEditedShape.reset();
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

		int shapeCount = 0;
		for (physics::Shape& shape : physics::getBodyShapes(rigidBodyId))
		{
			if (shape.isMarkedForDeletetion())
				continue;

			const auto shapeId = shape.getShapeId();

			ImGui::PushFont(fontSans);
			std::string shapeName = "Shape " + std::to_string(shapeCount++);
			ImGui::PushID(shapeName.c_str());
			if (ImGui::Button(shapeName.c_str()))
				lastEditedShape = shapeId;
			ImGui::PopFont();

			
			float currentDensity = shape.getDensity();
			const float oldDensity = currentDensity;
			if (ImGui::InputFloat("Density", &currentDensity))
			{
				lastEditedShape = shapeId;
				const float newDensity = currentDensity;		
				shape.setDensity(newDensity);

				timemachine::createSnapshot(
					[rigidBodyId, shapeId, oldDensity]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setDensity(oldDensity);
					},
					[rigidBodyId, shapeId, newDensity]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setDensity(newDensity);
					}
				);
			}


			float currentFriction = shape.getFriction();
			const float oldFriction = currentFriction;
			if (ImGui::InputFloat("Friction", &currentFriction))
			{
				lastEditedShape = shapeId;
				const float newFriction = currentFriction;
				shape.setFriction(newFriction);

				timemachine::createSnapshot(
					[rigidBodyId, shapeId, oldFriction]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setFriction(oldFriction);
					},
					[rigidBodyId, shapeId, newFriction]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setFriction(newFriction);
					}
				);
			}

			float currentRestitution = shape.getRestitution();
			const float oldRestitution = currentRestitution;
			if (ImGui::InputFloat("Restitution", &currentRestitution))
			{
				lastEditedShape = shapeId;
				const float newRestitution = currentRestitution;
				shape.setRestitution(newRestitution);

				timemachine::createSnapshot(
					[rigidBodyId, shapeId, oldRestitution]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setRestitution(oldRestitution);
					},
					[rigidBodyId, shapeId, newRestitution]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setRestitution(newRestitution);
					}
				);
			}

			math::vec3 currentPosition = shape.getPosition();
			ImGui::InputFloat3("Local position", &currentPosition.x);
			if (ImGui::IsItemDeactivated())
			{
				lastEditedShape = shapeId;
				currentPosition.x = std::clamp(currentPosition.x, -30.0f, 30.0f);
				currentPosition.y = std::clamp(currentPosition.y, -30.0f, 30.0f);
				currentPosition.z = std::clamp(currentPosition.z, -30.0f, 30.0f);

				const auto oldPos = shape.getPosition();
				const auto newPos = currentPosition;
				shape.setPosition(newPos);

				timemachine::createSnapshot(
					[rigidBodyId, shapeId, oldPos]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setPosition(oldPos);
					},
					[rigidBodyId, shapeId, newPos]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setPosition(newPos);
					}
				);
			}

			math::vec3 currentRotation = shape.getRotation();
			if (ImGui::InputFloat3("Local rotation", &currentRotation.x))
			{
				lastEditedShape = shapeId;
				currentRotation.x = std::clamp(currentRotation.x, -179.0f, 179.0f);
				currentRotation.y = std::clamp(currentRotation.y, -179.0f, 179.0f);
				currentRotation.z = std::clamp(currentRotation.z, -179.0f, 179.0f);

				const auto oldRot = shape.getRotation();
				const auto newRot = currentRotation;
				shape.setRotation(newRot);

				timemachine::createSnapshot(
					[rigidBodyId, shapeId, oldRot]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setRotation(oldRot);
					},
					[rigidBodyId, shapeId, newRot]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).setRotation(newRot);
					}
				);
			}

			auto editLBH = [](
					const physics::RigidBodyId rigidBodyId,
					const physics::RigidBodyShapeId shapeId
					)
			{
				auto& shape = physics::getBodyShape(rigidBodyId, shapeId);
				switch (shape.getShapeType())
				{
				case (physics::ShapeType::BOX): 
				{
					auto halfExtents = shape.getBoxHalfExtents();
					if (ImGui::InputFloat3(
						    "Half dimensions",
						    &halfExtents.x)
					)
					{
						halfExtents.x = std::clamp(
							halfExtents.x,
							0.1f,
							30.0f
						);

						halfExtents.y = std::clamp(
							halfExtents.y,
							0.1f,
							30.0f
						);

						halfExtents.z = std::clamp(
							halfExtents.z,
							0.1f,
							30.0f
						);

						const auto oldExtents = shape.getBoxHalfExtents();
						shape.setBoxHalfExtents(halfExtents);

						timemachine::createSnapshot(
							[rigidBodyId, shapeId, oldExtents] {
								physics::getBodyShape(
										rigidBodyId, 
										shapeId).setBoxHalfExtents(oldExtents);
							},
							[rigidBodyId, shapeId, halfExtents] {
								physics::getBodyShape(
										rigidBodyId, 
										shapeId).setBoxHalfExtents(halfExtents);
							}
						);
					}
					break;
				}

				case (physics::ShapeType::SPHERE):
				case (physics::ShapeType::CAPSULE):
				case (physics::ShapeType::CYLINDER): {
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

			// auto editRadius = [](physics::Shape& shape,
			// 		     physics::ShapeDef& shapeData,
			// 		     const physics::ShapeType type) -> void {
			// 	switch (type)
			// 	{
			// 	case (physics::ShapeType::SPHERE): {
			// 		if (ImGui::InputFloat("Radius", &shapeData.radius))
			// 		{
			// 			shapeData.radius =
			// 				std::clamp(shapeData.radius, 0.1f, 30.0f);

			// 			shape.setSphereRadius(shapeData.radius);
			// 		}
			// 	}
			// 	case (physics::ShapeType::CAPSULE):
			// 	case (physics::ShapeType::CYLINDER): {
			// 		auto radius = shape.getCapsuleRadius();
			// 		ImGui::SliderFloat("Radius", &radius, 0.1f, 30.0f, "%.2f");
			// 		if (ImGui::IsItemDeactivated())
			// 		{
			// 			shape.setCapsuleRadius(radius);
			// 		}

			// 		break;
			// 	}

			// 	default: {

			// 		CLZ_ASSERT(
			// 			false,
			// 			"Unknown shape type passed "
			// 			"while requesting LBH"
			// 		);
			// 	}
			// 	}
			// };

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
				
				editLBH(rigidBodyId, shapeId);
			}
			case (physics::ShapeType::SPHERE): {
				ImGui::PushFont(fontMono);
				ImGui::Text(
					"Shape type: "
				);
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::PushFont(fontMonoBold);
				ImGui::Text("Box");
				ImGui::PopFont();

				// editRadius(shape, shapeData, physics::ShapeType::SPHERE);

				break;
			}
			case (physics::ShapeType::CAPSULE): 
				ImGui::PushFont(fontMono);
				ImGui::Text("Shape type: ");
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::PushFont(fontMonoBold);
				ImGui::Text("Capsule");
				ImGui::PopFont();

				// editLBH(rigidBodyId, i, physics::ShapeType::CAPSULE);
				// editRadius(shape, shapeData, physics::ShapeType::CAPSULE);

			case (physics::ShapeType::CYLINDER): 
				ImGui::PushFont(fontMono);
				ImGui::Text("Shape type: ");
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::PushFont(fontMonoBold);
				ImGui::Text("Capsule");
				ImGui::PopFont();

				// editLBH(rigidBodyId, i, physics::ShapeType::CYLINDER);
				// editRadius(shape, shapeData, physics::ShapeType::CYLINDER);

			}

			bool isSensor = shape.isSensor();
			const bool oldIsSensor = isSensor;
			if (ImGui::Checkbox("Is Sensor", &isSensor))
			{
				lastEditedShape = shapeId;
				shape.makeSensor(isSensor);

				const bool newIsSensor = isSensor;
				timemachine::createSnapshot(
					[rigidBodyId, shapeId, oldIsSensor]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).makeSensor(oldIsSensor);
					},
					[rigidBodyId, shapeId, newIsSensor]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).makeSensor(newIsSensor);
					}
				);

			}

			bool enableSensorEvents = shape.areSensorEventsEnabled();
			const bool oldEnableSensorEvents = enableSensorEvents;
			if (ImGui::Checkbox("Enable Sensor Events", &enableSensorEvents))
			{
				lastEditedShape = shapeId;
				shape.enableSensorEvents(enableSensorEvents);

				const bool newEnableSensorEvents = enableSensorEvents;
				timemachine::createSnapshot(
					[rigidBodyId, shapeId, oldEnableSensorEvents]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).enableSensorEvents(oldEnableSensorEvents);
					},
					[rigidBodyId, shapeId, newEnableSensorEvents]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).enableSensorEvents(newEnableSensorEvents);
					}
				);
			}

			bool enableContactEvents = shape.areContactEventsEnabled();
			const bool oldEnableContactEvents = enableContactEvents;
			if (ImGui::Checkbox("Enable Contact Events", &enableContactEvents))
			{
				lastEditedShape = shapeId;
				shape.enableContactEvents(enableContactEvents);

				const bool newEnableContactEvents = enableContactEvents;
				timemachine::createSnapshot(
					[rigidBodyId, shapeId, oldEnableContactEvents]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).enableContactEvents(oldEnableContactEvents);
					},
					[rigidBodyId, shapeId, newEnableContactEvents]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).enableContactEvents(newEnableContactEvents);
					}
				);
			}

			bool enableHitEvents = shape.areHitEventsEnabled();
			const bool oldEnableHitEvents = enableHitEvents;
			if (ImGui::Checkbox("Enable Hit Events", &enableHitEvents))
			{
				lastEditedShape = shapeId;
				shape.enableHitEvents(enableHitEvents);

				const bool newEnableHitEvents = enableHitEvents;
				timemachine::createSnapshot(
					[rigidBodyId, shapeId, oldEnableHitEvents]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).enableHitEvents(oldEnableHitEvents);
					},
					[rigidBodyId, shapeId, newEnableHitEvents]()
					{
						physics::getBodyShape(rigidBodyId, shapeId).enableHitEvents(newEnableHitEvents);
					}
				);
			}



			const auto deleteShape = "Delete Shape";
			if (ImGui::Button(deleteShape))
			{
				ImGui::OpenPopup(deleteShape);
			}
			if (ImGui::BeginPopup(deleteShape))
			{
				if (ImGui::MenuItem("Sure ??"))
				{
					shape.markForDeletion();

					timemachine::createSnapshot(
						[rigidBodyId, shapeId] {
							physics::getBodyShape(
								rigidBodyId, 
								shapeId).unMarkForDeletion();
						},
						[rigidBodyId, shapeId] {
							physics::getBodyShape(
								rigidBodyId, 
								shapeId).markForDeletion();
						}
					);
				}
				if (ImGui::MenuItem("Nope"))
				{
					///
				}

				ImGui::EndPopup();
			}

			ImGui::Separator();
			ImGui::PopID();

		}


		ImGui::End();
	}

	void showBodySceneWindow()
	{
		if (!capturedEntity.has_value())
			return;

		ImGui::Begin(
			"RigidBody Shape Editor"
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
				renderer::updateCamera(Id);
			}

			if (rightClickThisFrame && !rightClickLastFrame)
			{
				window::disableCursor();
			}
			else if (!rightClickThisFrame && rightClickLastFrame)
			{
				window::enableCursor();
			}

			rightClickLastFrame = rightClickThisFrame;
		}

		const ImVec2 cursorPosBefore = ImGui::GetCursorScreenPos();

		ImGui::Image((ImTextureID)physicsBodyShapeImage.descriptorSet, avail);

		if (lastEditedShape.has_value())
		{
			const Rect2D rect{
				.x = static_cast<uint32_t>(cursorPosBefore.x),
				.y = static_cast<uint32_t>(cursorPosBefore.y),
				.width = physicsBodyShapeImage.extent.width,
				.height = physicsBodyShapeImage.extent.height};

			const auto rigidBodyId = ecs::getComponent<physics::RigidBodyComponent>(
							capturedEntity.value()).rigidBodyId;
			const physics::RigidBodyShapeId shapeId = lastEditedShape.value();
			auto& shape = physics::getBodyShape(rigidBodyId, lastEditedShape.value());
			ecs::EditorTransformComponent shapeTransform;	
			shapeTransform.position = shape.getPosition();
			shapeTransform.rotation = shape.getRotation();
			shapeTransform.scale = shape.getBoxHalfExtents() * 2;
			
			auto proj = renderer::getCameraProjMatrix(physicsBodyShapeImage.cameraId, rect.width, rect.height);
			const auto view = renderer::getCameraViewMatrix(physicsBodyShapeImage.cameraId);
			const auto [newTransform, newEditorTransform, change] = gizmoTransform(rect, proj, view, shapeTransform);

			static auto prevTransform      = shapeTransform;
			static bool gizmoUsedThisFrame = false;
			static bool gizmoUsedLastFrame = false;
			if (change)
			{
				gizmoUsedThisFrame = true;
				if (!gizmoUsedLastFrame)
					prevTransform = shapeTransform;

				shape.setPosition(newEditorTransform.position);
				shape.setRotation(newEditorTransform.rotation);

				switch (shape.getShapeType())
				{
				case physics::ShapeType::BOX:
					shape.setBoxHalfExtents(newEditorTransform.scale * 0.5f);
					break;

				case physics::ShapeType::SPHERE:
				case physics::ShapeType::CYLINDER:
				case physics::ShapeType::CAPSULE:
					break;
				}
			}
			else
			{
				gizmoUsedThisFrame = false;
				if (gizmoUsedLastFrame)
				{
					ecs::EditorTransformComponent oldTransform = prevTransform;
					timemachine::createSnapshot(
						[rigidBodyId, shapeId, oldTransform]
						{
							auto& shape = physics::getBodyShape(rigidBodyId, shapeId);
							shape.setPosition(oldTransform.position);
							shape.setRotation(oldTransform.rotation);
							shape.setBoxHalfExtents(oldTransform.scale * 0.5f);
						},
						[rigidBodyId, shapeId, newEditorTransform]
						{
							auto& shape = physics::getBodyShape(rigidBodyId, shapeId);
							shape.setPosition(newEditorTransform.position);
							shape.setRotation(newEditorTransform.rotation);
							shape.setBoxHalfExtents(newEditorTransform.scale * 0.5f);
						}
					);
					
				}
			}

			gizmoUsedLastFrame = gizmoUsedThisFrame;

		}
		ImGui::End();
	}


	void drawBodyEditorOffscreenImage(VkCommandBuffer commandBuffer)
	{
		if (!capturedEntity.has_value())
		{
			return;
		}
		prepareOffscreenTarget(physicsBodyShapeImage);

		/// over here, src is set to ~fragment shader,
		/// cuz imgui also has to read this image that frame
		/// dn wanna mess with that at all.
		renderer::transition_image_layout(
			physicsBodyShapeImage.image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_2_SHADER_READ_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
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
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderingInfo.pNext = nullptr, renderingInfo.flags = 0;
		renderingInfo.renderArea = {{0, 0}, physicsBodyShapeImage.extent};
		renderingInfo.layerCount = 1, renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
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
			renderer::textureDescriptorSets[renderer::r_currentFrame]
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

		if (ecs::hasComponent<renderer::ModelComponent>(
				capturedEntity.value()))
		{
			renderer::drawModel(
				ecs::getComponent<renderer::ModelComponent>(
					capturedEntity.value()
				).modelId,
				math::vec3(0.0f),
				math::quat(1.0f, 0.0f, 0.0f, 0.0f),
				ecs::getComponent<ecs::TransformComponent>(capturedEntity.value())
					.scale,
				commandBuffer
			);
		}

		const auto& shapes = physics::getBodyShapes(
			ecs::getComponent<physics::RigidBodyComponent>(
				capturedEntity.value()
			).rigidBodyId
		);

		for (const auto& shapeData : shapes)
		{
			if (shapeData.isMarkedForDeletetion())
				continue;

			const auto quat = math::quatFromEuler(math::radians(shapeData.getRotation()));
			const auto pos = shapeData.getPosition();
			math::vec3 scale;
			renderer::Shape drawShape;
			switch (shapeData.getShapeType())
			{
			case physics::ShapeType::BOX: {
				scale = shapeData.getBoxHalfExtents() * 2;
				drawShape = renderer::Shape::BOX;
				break;
			}

			case physics::ShapeType::SPHERE:
				drawShape = renderer::Shape::BOX;
				break;
			case physics::ShapeType::CAPSULE:
				drawShape = renderer::Shape::BOX;
				break;
			case physics::ShapeType::CYLINDER:
				drawShape = renderer::Shape::BOX;
				break;
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
