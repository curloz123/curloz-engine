/**
 * @file rigidbodycomponent.cpp
 * @author curl0z
 * @brief displays rigid component's data n all
 * in inspector. Internally can change rigid body's
 * data and body's attributes.
 */

#include "../../include/inspector/rigidbodycomponent.hpp"
#include "../../include/editor_types.hpp"
#include "../../include/offscreen/backend/pipeline.hpp"
#include "../../include/offscreen/backend/descriptor.hpp"
#include "../../include/offscreen/backend/ubo.hpp"
#include "../../include/offscreen/camera.hpp"
#include "../../include/timemachine.hpp"
#include "core/logs.hpp"
#include "imgui.h"
#include "include/offscreen/offscreentarget.hpp"
#include "math/quateulerconv.hpp"
#include "math/worldtransform.hpp"
#include "physics/shape.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/pipelinedata/descriptor.hpp"
#include "renderer/model/model.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "renderer/shapes.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/vk_types.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"
#include "window/inputmanager.hpp"

namespace clz::editor
{
	/// @brief Stores previous data in rigid body data component
	ecs::BodyComponent previousBodyData;
	/// @brief Stores previous current in rigid body data component
	ecs::BodyComponent currentRigidBodyData;

	/**
	 * @brief Shows rigid body header in inspector
	 * Allows to edit physics-body data of current selected entity
	 * Opens a new window for shape editor
	 * @note It currently in-complete. Needs undo/redo work-done
	 */
	void showRigidBodyHeader()
	{
		if (!ImGui::CollapsingHeader("RigidBody"))
			return;

		const auto& bodyId = ecs::getComponent<ecs::BodyComponent>(currentSelectedEntity.value()).bodyId;
		const auto& boxShapes = ecs::getComponent<ecs::ShapeComponent>(currentSelectedEntity.value()).boxShapes;


		ImGui::PushFont(fontMonoBold);
		std::string type = "undefined";
		const auto& bodyType = physics::getBodyType(bodyId);
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
			if (ImGui::MenuItem("Static"))
			{
				physics::setBodyType(bodyId, physics::BodyType::StaticBody);
			}
			if (ImGui::MenuItem("Kinematic"))
			{
				physics::setBodyType(bodyId, physics::BodyType::KinematicBody);
			}
			if (ImGui::MenuItem("Dynamic"))
			{
				physics::setBodyType(bodyId, physics::BodyType::DynamicBody);
			}

			ImGui::EndPopup();
		}


		ImGui::PushFont(fontMono);
		ImGui::Separator();
		ImGui::Text("Computed Mass: %.2f", physics::getBodyMass(bodyId));
		ImGui::Text("Mass is computed via\ndensity of shapes attached to it");
		ImGui::Separator();

		bool enterCommit = false;
		bool defocusCommit = false;

		float linearDamping = physics::getBodyLinearDamping(bodyId);
		ImGui::Text("Linear Damping");
		enterCommit = ImGui::InputFloat("##lineardamping: ", &linearDamping, 0.0f, 1.0f,
			"%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
		physics::setBodyLinearDamping(bodyId, linearDamping);

		float angularDamping = physics::getBodyAngularDamping(bodyId);
		ImGui::Text("Angular Damping");
		enterCommit = ImGui::InputFloat("##angulardamping: ", &angularDamping, 0.0f, 1.0f,
			"%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
		physics::setBodyAngularDamping(bodyId, angularDamping);

		bool sleepEnabled = physics::isSleepEnabled(bodyId);
		ImGui::Text("Sleeping");
		if (ImGui::Checkbox("##sleeping", &sleepEnabled))
		{
			physics::enableSleep(bodyId, sleepEnabled);
		}

		auto linearLocks = physics::getBodyLinearLocks(bodyId);
		ImGui::Text("Linear lock");
		if (ImGui::Checkbox("X##xlinear", &linearLocks[0]))
		{
			physics::setBodyLinearLocks(bodyId, linearLocks);
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Y##ylinear", &linearLocks[1]))
		{
			physics::setBodyLinearLocks(bodyId, linearLocks);
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Z##zlinear", &linearLocks[2]))
		{
			physics::setBodyLinearLocks(bodyId, linearLocks);
		}

		auto angularLocks = physics::getBodyAngularLocks(bodyId);
		ImGui::Text("Angular lock");
		if (ImGui::Checkbox("X##xangular", &angularLocks[0]))
		{
			physics::setBodyAngularLocks(bodyId, angularLocks);
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Y##yangular", &angularLocks[1]))
		{
			physics::setBodyAngularLocks(bodyId, angularLocks);
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Z##zangular", &angularLocks[2]))
		{
			physics::setBodyAngularLocks(bodyId, angularLocks);
		}

		if ((ImGui::Button("Add Shape") && !physicsBodyShapeImage.showTarget))
		{
			camera::resetCamera();
			physicsBodyShapeImage.showTarget = true;
		}

		ImGui::PopFont();

	}

	void presentBodyEditorWindow()
	{
		const auto bodyId = ecs::getComponent<ecs::BodyComponent>(currentSelectedEntity.value()).bodyId;
		auto& shapeContainer = ecs::getComponent<ecs::ShapeComponent>(
								currentSelectedEntity.value()).boxShapes;

		ImGui::Begin("Shape Controls");

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
				physics::BoxShape boxShape{};
				newShapes.emplace_back(boxShape);
				anyChanges = true;
				clz::log::debug("Added box shape, click save to apply changes");
			}
			ImGui::EndPopup();
		}


		ImGui::SameLine();
		if (ImGui::Button("Save") && anyChanges)
		{
			for (const auto index : changedShapesIndex)
			{
				physics::BoxShape boxShape = physics::BoxShape(changedShapes[index].halfDimensions,
					changedShapes[index].position, changedShapes[index].rotation,
					changedShapes[index].density, changedShapes[index].friction, changedShapes[index].restitution);

				physics::modifyShapeByIndex(bodyId, boxShape, shapeContainer, index);
				clz::log::debug("new density: " + std::to_string(changedShapes[index].density));
			}
			for (auto& newShape : newShapes)
			{
				physics::BoxShape boxShape = physics::BoxShape(newShape.halfDimensions,
					newShape.position, newShape.rotation,
					newShape.density, newShape.friction, newShape.restitution);

				physics::attachShapeToBody(bodyId, shapeContainer, boxShape);
			}

			changedShapes.clear();
			changedShapesIndex.clear();
			newShapes.clear();
			anyChanges = false;

			clz::log::debug("Saved changes");
		}

		if (!anyChanges)
		{
			changedShapes = shapeContainer;
		}
		ImGui::Separator();
		ImGui::PushFont(fontMonoBold);
		ImGui::Text("Attached box shapes");
		ImGui::PopFont();

		for (size_t i = 0; i < changedShapes.size(); i++)
		{
			ImGui::PushFont(fontSans);
			std::string shapeName = "Shape " + std::to_string(i);
			if (shapeContainer[i].position != changedShapes[i].position ||
				shapeContainer[i].rotation != changedShapes[i].rotation ||
				shapeContainer[i].halfDimensions != changedShapes[i].halfDimensions ||
				shapeContainer[i].density != changedShapes[i].density ||
				shapeContainer[i].friction != changedShapes[i].friction ||
				shapeContainer[i].restitution != changedShapes[i].restitution)
			{
				shapeName += "*";
			}
			ImGui::PushID(shapeName.c_str());
			ImGui::Text(shapeName.c_str());
			ImGui::PopFont();


			if (ImGui::InputFloat("Density", &changedShapes[i].density, 0.01f, 100.0f, "%.2f"))
			{
				anyChanges = true;
			}
			if (ImGui::IsItemDeactivated())
			{
				changedShapesIndex.insert(i);
			}
			if (ImGui::InputFloat("Friction", &changedShapes[i].friction, 0.01f, 100.0f, "%.2f"))
			{
				anyChanges = true;
			}
			if (ImGui::IsItemDeactivated())
			{
				changedShapesIndex.insert(i);
			}
			if (ImGui::InputFloat("Restitution", &changedShapes[i].restitution, 0.01f, 1.0f, "%.2f"))
			{
				anyChanges = true;
			}
			if (ImGui::IsItemDeactivated())
			{
				changedShapesIndex.insert(i);
			}

			if (ImGui::SliderFloat3("Local position", &changedShapes[i].position.x, 0.1f, 30.0f, "%.2f"))
			{
				anyChanges = true;
			}
			if (ImGui::IsItemDeactivated())
			{
				changedShapesIndex.insert(i);
			}

			if (ImGui::SliderFloat3("Local rotation", &changedShapes[i].rotation.x, -179.99f, 179.99f, "%.2f"))
			{
				anyChanges = true;
			}
			if (ImGui::IsItemDeactivated())
			{
				changedShapesIndex.insert(i);
			}

			if (ImGui::SliderFloat3("Half dimensions", &changedShapes[i].halfDimensions.x, 0.1f, 30.0f, "%.2f"))
			{
				anyChanges = true;
			}
			if (ImGui::IsItemDeactivated())
			{
				changedShapesIndex.insert(i);
			}

			ImGui::PopID();
		}
		ImGui::Separator();
		ImGui::Text("New box shapes");
		for (size_t i = 0; i < newShapes.size(); i++)
		{
			ImGui::PushFont(fontSans);
			std::string shapeName = "Shape " + std::to_string(i + changedShapes.size());
			ImGui::PushID(shapeName.c_str());
			ImGui::Text(shapeName.c_str());
			ImGui::PopFont();


			ImGui::SliderFloat("Density", &newShapes[i].density, 0.01f, 100.0f, "%.2f");
			ImGui::SliderFloat("Friction", &newShapes[i].friction, 0.01f, 100.0f, "%.2f");
			ImGui::SliderFloat3("Local position", &newShapes[i].position.x, -30.0f, 30.0f, "%.2f");
			ImGui::SliderFloat3("Local rotation", &newShapes[i].rotation.x, -179.99f, 179.99f, "%.2f");
			ImGui::SliderFloat3("Half dimensions", &newShapes[i].halfDimensions.x, 0.1f, 30.0f, "%.2f");
			ImGui::PopID();
		}

		ImGui::End();


		ImGui::Begin("RigidBody Shape Editor");
		const char* closeWindow = "Close window";
		if (ImGui::Button(closeWindow))
		{
			ImGui::OpenPopup("Close window");
		}
		if (ImGui::BeginPopup(closeWindow))
		{
			if (ImGui::MenuItem("Sure"))
			{
				physicsBodyShapeImage.showTarget = false;
				camera::resetCamera();
				ImGui::EndPopup();
				ImGui::End();
				clz::log::debug("Exiting, hope you saved everything");
				return;
			}
			if (ImGui::MenuItem("Not yet"))
			{
				clz::log::debug("Not exiting");
				//
			}
			ImGui::EndPopup();
		}
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
			recreateOffscreenTarget(physicsBodyShapeImage, width, height);
			clz::log::debug("Won't show image this frame, resizing rn");
			ImGui::End();
			return;
		}

		ImGui::Image((ImTextureID)physicsBodyShapeImage.descriptorSet, avail);

		ImGui::End();

	}

	void drawBodyEditorOffscreenImage(VkCommandBuffer commandBuffer)
	{
		renderer::transition_image_layout(physicsBodyShapeImage.image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			0, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			VK_IMAGE_ASPECT_COLOR_BIT, commandBuffer);

		VkRenderingAttachmentInfoKHR colorAttachment = {};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachment.pNext = nullptr;
		colorAttachment.imageView = physicsBodyShapeImage.imageView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {
			.color = {
				.float32 = {0.0f, 0.0f, 0.0f, 1.0f}
			}
		};

		VkRenderingAttachmentInfoKHR depthAttachment = {};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthAttachment.imageView = physicsBodyShapeImage.depthImageView;
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.clearValue.depthStencil.depth = 1.0f;

		VkRenderingInfoKHR renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		renderingInfo.pNext = nullptr,
		renderingInfo.flags = 0,
		renderingInfo.renderArea = {{0, 0}, physicsBodyShapeImage.extent},
		renderingInfo.layerCount = 1,
		renderingInfo.colorAttachmentCount = 1,
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
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backend::editorPipelineContext.pipeline);
		// Update uniform buffers here

		camera::update();
		auto p = math::makePerspectiveMatrix(camera::Far, camera::Near,
				viewport.width / viewport.height, math::radians(camera::Fov));
		auto v = math::makeViewMatrix(camera::Position, camera::Position + camera::localFront, camera::WorldUp);
		auto ubo = backend::CameraShaderUBO{
			.projection = p,
			.view = v
		};
		memcpy(backend::editorCameraUBO.mapped[renderer::r_currentFrame], &ubo, sizeof(ubo));

		const std::array descriptorSets = {
			backend::cameraDescriptorSets [renderer::r_currentFrame],	// binding set is 1
			renderer::samplerDescriptorSets[renderer::r_currentFrame]	// As sampler's binding set is 0
		};
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backend::editorPipelineContext.layout,
			0, descriptorSets.size(), descriptorSets.data(),
			0, nullptr);

		renderer::drawModel(ecs::getComponent<ecs::ModelComponent>(currentSelectedEntity.value()).modelId,
			math::vec3(0.0f), math::quat(), math::vec3(1.0f),
			commandBuffer);


		if (!anyChanges)
		{
			const auto& boxShapes =
				ecs::getComponent<ecs::ShapeComponent>(currentSelectedEntity.value()).boxShapes;
			for (const auto& boxShape : boxShapes)
			{
				const auto quat = math::quatFromEuler(boxShape.rotation);
				const auto pos = boxShape.position;
				const auto scale = boxShape.halfDimensions * 2;
				renderer::drawShape(commandBuffer, renderer::Shape::BOX, p, v,
					math::getModelMatrix(quat, pos, scale),
					math::vec4(0.0f, 0.5f, 0.0f, 1.0f));

			}
		}
		else
		{
			for (const auto& shape : changedShapes)
			{
				const auto quat = math::quatFromEuler(shape.rotation);
				const auto pos = shape.position;
				const auto scale = shape.halfDimensions * 2;
				renderer::drawShape(commandBuffer, renderer::Shape::BOX, p, v,
					math::getModelMatrix(quat, pos, scale),
					math::vec4(0.5f, 0.5f, 0.0f, 1.0f));
			}
			for (const auto& boxShape : newShapes)
			{
				const auto quat = math::quatFromEuler(boxShape.rotation);
				const auto pos = boxShape.position;
				const auto scale = boxShape.halfDimensions * 2;
				renderer::drawShape(commandBuffer, renderer::Shape::BOX, p, v,
					math::getModelMatrix(quat, pos, scale),
					math::vec4(0.0f, 0.0f, 0.5f, 1.0f));
			}
		}

		vkCmdEndRendering(commandBuffer);
		renderer::transition_image_layout(physicsBodyShapeImage.image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR, 0,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT, commandBuffer);
	}
} // namespace clz::editor
