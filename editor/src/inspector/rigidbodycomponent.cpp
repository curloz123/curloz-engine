/**
 * @file rigidbodycomponent.cpp
 * @author curl0z
 * @brief displays rigid component's data n all
 * in inspector. Internally can change rigid body's
 * data and body's attributes.
 */

#include "../../include/inspector/rigidbodycomponent.hpp"
#include "math/quateulerconv.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "../../include/offscreen/backend/pipeline.hpp"
#include "renderer/pipelineinput/mainpipeline.hpp"
#include "../../include/editor_types.hpp"
#include "renderer/entitydata/rendermodel.hpp"
#include "../../include/offscreen/camera.hpp"
#include "../../include/timemachine.hpp"
#include "core/logs.hpp"
#include "imgui.h"
#include "include/offscreen/offscreentarget.hpp"
#include "math/worldtransform.hpp"
#include "window/inputmanager.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/vk_types.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"
#include "renderer/shapes.hpp"
#include "physics/shape.hpp"

namespace clz::editor
{
	/// @brief Stores previous data in rigid body data component
	ecs::RigidBodyDataComponent previousRigidBodyData;
	/// @brief Stores previous current in rigid body data component
	ecs::RigidBodyDataComponent currentRigidBodyData;

	bool showShapeWindow = false;

	/**
	 * @brief Shows rigid body header in inspector
	 * also stores a snapshot upon every change
	 */
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

		if ((ImGui::Button("Add Shape") && !physicsBodyShapeImage.showTarget))
		{
			camera::resetCamera();
			physicsBodyShapeImage.showTarget = true;
		}

		ImGui::PopFont();


		if (anyEditFinished)
		{
			ecs::setComponent<ecs::RigidBodyDataComponent>(currentSelectedEntity.value(), currentRigidBodyData);

			clz::log::debug("Creating a snapshot of rigidbody component data");
			createSnapshot<ecs::RigidBodyDataComponent>(currentSelectedEntity.value(), previousRigidBodyData, currentRigidBodyData);
		}
	}

	void presentBodyEditorWindow()
	{
		const auto& shapes =
			ecs::getComponent<ecs::RigidBodyDataComponent>(currentSelectedEntity.value()).boxShapes;

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
				changedShapes = shapes;
				anyChanges = true;
				clz::log::debug("Added box shape, click save to apply changes");
			}
			ImGui::EndPopup();
		}


		ImGui::SameLine();
		if (ImGui::Button("Save") && anyChanges)
		{
			for (auto index : changedShapesIndex)
			{
				physics::BoxShape boxShape = physics::BoxShape(changedShapes[index].halfDimensions,
					changedShapes[index].position, changedShapes[index].rotation);

				physics::modifyShapeByIndex(currentSelectedEntity.value(),
					boxShape, index);

			}
			for (auto& newShape : newShapes)
			{
				physics::BoxShape boxShape = physics::BoxShape(newShape.halfDimensions,
					newShape.position, newShape.rotation);

				physics::attachShapeToBody(currentSelectedEntity.value(),
					boxShape);
			}

			changedShapes.clear();
			changedShapesIndex.clear();
			newShapes.clear();
			anyChanges = false;

			clz::log::debug("Saved changes");
		}


		if (!anyChanges)
		{
			changedShapes = shapes;
		}
		ImGui::Separator();
		ImGui::PushFont(fontMonoBold);
		ImGui::Text("Attached box shapes");
		ImGui::PopFont();

		for (size_t i = 0; i < changedShapes.size(); i++)
		{
			ImGui::PushFont(fontSans);
			std::string shapeName = "Shape " + std::to_string(i);
			if (shapes[i].position != changedShapes[i].position ||
				shapes[i].rotation != changedShapes[i].rotation ||
				shapes[i].halfDimensions != changedShapes[i].halfDimensions)
			{
				shapeName += "*";
			}
			ImGui::Text(shapeName.c_str());
			ImGui::PopFont();


			ImGui::SliderFloat3("Local position", &changedShapes[i].position.x, 0.1f, 30.0f, "%.2f");
			if (ImGui::IsItemActivated())
				anyChanges = true;
			if (ImGui::IsItemDeactivated())
				changedShapesIndex.insert(i);

			ImGui::SliderFloat3("Local rotation ", &changedShapes[i].rotation.x, -179.99f, 179.99f, "%.2f");
			if (ImGui::IsItemActivated())
				anyChanges = true;
			if (ImGui::IsItemDeactivated())
				changedShapesIndex.insert(i);

			ImGui::SliderFloat3("Half dimensions", &changedShapes[i].halfDimensions.x, 0.1f, 30.0f, "%.2f");
			if (ImGui::IsItemActivated())
				anyChanges = true;
			if (ImGui::IsItemDeactivated())
				changedShapesIndex.insert(i);
		}
		ImGui::Separator();
		ImGui::Text("New box shapes");
		for (size_t i = 0; i < newShapes.size(); i++)
		{
			ImGui::PushFont(fontSans);
			std::string shapeName = "Shape " + std::to_string(i + changedShapes.size());
			ImGui::Text(shapeName.c_str());
			ImGui::PopFont();


			ImGui::SliderFloat3("Local position", &newShapes[i].position.x, 0.1f, 30.0f, "%.2f");
			ImGui::SliderFloat3("Local rotation ", &newShapes[i].rotation.x, -179.99f, 179.99f, "%.2f");
			ImGui::SliderFloat3("Half dimensions", &newShapes[i].halfDimensions.x, 0.1f, 30.0f, "%.2f");
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
		colorAttachment.clearValue = {{{1.0f, 0.0f, 0.0f, 1.0f}}};

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
		memcpy(backend::CameraUBO.mapped[renderer::r_currentFrame], &ubo, sizeof(ubo));
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backend::editorPipelineContext.layout,
			0, 1, &backend::editorPipelineContext.descriptorSets[renderer::r_currentFrame],
			0, nullptr);

		renderer::renderEntity(commandBuffer,
			ecs::getComponent<ecs::ModelComponent>(currentSelectedEntity.value()).modelID,
			math::mat4(1.0f));


		if (!anyChanges)
		{
			const auto& shapes =
				ecs::getComponent<ecs::RigidBodyDataComponent>(currentSelectedEntity.value());
			for (const auto& boxShape : shapes.boxShapes)
			{
				const auto quat = math::quatFromEuler(boxShape.rotation);
				const auto pos = boxShape.position;
				const auto scale = boxShape.halfDimensions * 2;
				renderer::drawShape(commandBuffer, renderer::Shape::BOX, p, v,
					math::getModelMatrix(quat, pos, scale),
					math::vec4(0.0f, 0.5f, 0.5f, 1.0f));

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
					math::vec4(0.0f, 0.5f, 0.5f, 1.0f));
			}
			for (const auto& boxShape : newShapes)
			{
				const auto quat = math::quatFromEuler(boxShape.rotation);
				const auto pos = boxShape.position;
				const auto scale = boxShape.halfDimensions * 2;
				renderer::drawShape(commandBuffer, renderer::Shape::BOX, p, v,
					math::getModelMatrix(quat, pos, scale),
					math::vec4(0.0f, 0.5f, 0.5f, 1.0f));
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
