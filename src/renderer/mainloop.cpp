/**
 * @file mainloop.cpp
 * @author curl0z
 * @brief Implementation of the main loop of the frame
 */

#include "renderer/mainloop.hpp"
#include "core/enginestate.hpp"
#include "core/logs.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/context/render_target_context.hpp"
#include "renderer/drawscene.hpp"
#include "renderer/model/model.hpp"
#include "renderer/postprocess/bloom.hpp"
#include "renderer/renderer.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/vk_types.hpp"
#include "window/mouse.hpp"
#include "renderer/postprocess/post_process.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/postprocess/bloom_sample.hpp"
#include <vulkan/vulkan_core.h>


#ifdef CLZ_ENABLE_EDITOR
#include "include/editor.hpp"
#include "include/sceneview.hpp"
#include "include/cross_system_flags.hpp"
#endif

namespace clz::renderer
{
	/// @copydoc
	void waitForGPU(VkFence fence)
	{
		if (vkWaitForFences(
			    renderer::r_deviceContext.device,
			    1,
			    &fence,
			    VK_TRUE,
			    UINT64_MAX
		    ) != VK_SUCCESS) [[unlikely]]
		{
			clz::log::error("failed to wait for fence");
		}
	}

	/// @copydoc
	void acquireNextImage(VkSemaphore semaphore, uint32_t& rImageIndex)
	{
		const VkResult acquireResult = vkAcquireNextImageKHR(
			r_deviceContext.device,
			r_swapchainContext.swapchain,
			UINT64_MAX,
			semaphore,
			VK_NULL_HANDLE,
			&rImageIndex
		);
		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR)
			[[unlikely]]
		{
			clz::log::debug("swapchain will be recreated");
			r_swapchainOutdated = true;
		}
		else if (acquireResult != VK_SUCCESS) [[unlikely]]
		{
			clz::log::error("renderer/mainloop: Failed to acquire next image");
		}
	}

	/// @copydoc
	void resetFence(VkFence fence)
	{
		if (vkResetFences(r_deviceContext.device, 1, &fence) != VK_SUCCESS) [[unlikely]]
		{
			clz::log::error("Failed to reset fence");
		}
	}

	/// @copydoc
	void startCommandBuffer(VkCommandBuffer commandBuffer)
	{
		if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS) [[unlikely]]
		{
			clz::log::error("Could not reset command buffer mid loop");
		}
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) [[unlikely]]
		{
			clz::log::error("renderer: midloop: Failed to begin command buffer");
		}
	}


	/// @copydoc recordCommandBuffer
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		/// @brief Renders the 3D scene into r_renderTargetContext.image.
		/// Shared by both Editor and Game paths -- the scene always draws
		/// into the same HDR render target regardless of mode; only where
		/// the final post-processed result ends up differs.
		auto drawSceneIntoRenderTarget = [commandBuffer]()
		{
			transition_image_layout(
				r_renderTargetContext.image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				0,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
				VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);
			transition_image_layout(
				r_renderTargetContext.msaaImage,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				0,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
				VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);


			VkRenderingAttachmentInfo colorAttachment = {};
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			colorAttachment.pNext = nullptr;
			colorAttachment.imageView = r_renderTargetContext.msaaImageView;
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.clearValue = {
				.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
			};
			colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
			colorAttachment.resolveImageView = r_renderTargetContext.imageView;
			colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


			VkRenderingAttachmentInfo depthAttachment = {};
			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			depthAttachment.imageView = r_renderTargetContext.depthImageView;
			depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.clearValue.depthStencil.depth = 1.0f;

			VkRenderingInfo renderingInfo = {};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
			renderingInfo.pNext = nullptr;
			renderingInfo.flags = 0;
			renderingInfo.renderArea = {{0, 0}, r_renderTargetContext.imageExtent};
			renderingInfo.layerCount = 1;
			renderingInfo.colorAttachmentCount = 1;
			renderingInfo.pColorAttachments = &colorAttachment;
			renderingInfo.pDepthAttachment = &depthAttachment;
			vkCmdBeginRendering(commandBuffer, &renderingInfo);

			const VkViewport viewport = {
				.x = 0.0f,
				.y = 0.0f,
				.width = static_cast<float>(r_renderTargetContext.imageExtent.width),
				.height = static_cast<float>(r_renderTargetContext.imageExtent.height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f,
			};
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			const VkRect2D scissor = {{0, 0}, r_renderTargetContext.imageExtent};
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			/// --- Main artist ---
			drawScene(commandBuffer);
			/// --- artist finished drawing ---

			vkCmdEndRendering(commandBuffer);

			// post_process::applyPreTonemapProcess samples this next via
			// the post-process descriptor set.
			transition_image_layout(
				r_renderTargetContext.image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
				VK_ACCESS_2_SHADER_READ_BIT_KHR,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);

		};

		/// @brief Copies post_process::postTonemapImage (left in
		/// TRANSFER_SRC_OPTIMAL by applyPostProcessing) into dstImage, then
		/// transitions dstImage into dstFinalLayout.
		auto copyPostprocessResultInto = [commandBuffer](
			VkImage dstImage,
			VkExtent2D dstExtent,
			VkImageLayout dstFinalLayout,
			VkPipelineStageFlags2 dstFinalStage,
			VkAccessFlags2 dstFinalAccess
		)
		{
			transition_image_layout(
				dstImage,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				0,
				VK_ACCESS_2_TRANSFER_WRITE_BIT,
				VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);

			copyImage2D(
				post_process::postTonemapImage,
				r_renderTargetContext.imageExtent,
				dstImage,
				dstExtent,
				commandBuffer
			);
			// copyImage2D(
			// 	post_process::verticalBloomImage.image,
			// 	r_renderTargetContext.imageExtent,
			// 	dstImage,
			// 	dstExtent,
			// 	commandBuffer
			// );

			transition_image_layout(
				dstImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				dstFinalLayout,
				VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
				dstFinalAccess,
				VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
				dstFinalStage,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);
		};

		/// @brief Draws Black into the image this frame
		/// Useful when images have been recreated this frame only
		auto drawNullScene = [commandBuffer](
			VkImage image,
			VkImageView imageView,
			VkExtent2D extent,
			VkImageLayout finalLayout,
			VkAccessFlags2 finalAccessMask,
			VkPipelineStageFlags2 finalStage
			)
		{
			transition_image_layout(
				image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				0,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);

			VkRenderingAttachmentInfo colorAttachment = {};
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.pNext = nullptr;
			colorAttachment.imageView = imageView;
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

			VkRenderingInfo renderingInfo = {};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderingInfo.renderArea = {{0, 0}, extent};
			renderingInfo.layerCount = 1;
			renderingInfo.colorAttachmentCount = 1;
			renderingInfo.pColorAttachments = &colorAttachment;

			vkCmdBeginRendering(commandBuffer, &renderingInfo);
			const VkViewport viewport = {
				.x = 0.0f,
				.y = 0.0f,
				.width = static_cast<float>(extent.width),
				.height = static_cast<float>(extent.height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f,
			};
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			const VkRect2D scissor = {{0, 0}, extent};
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			vkCmdEndRendering(commandBuffer);

			transition_image_layout(
				image,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				finalLayout,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				finalAccessMask,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				finalStage,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);
		};

		/*
		drawNullScene();
		goto endCommandBuffer;
		*/

		if (r_swapchainOutdated) [[unlikely]]
		{
			drawNullScene(
				r_swapchainContext.images[imageIndex],
				r_swapchainContext.imageViews[imageIndex],
				r_swapchainContext.extent,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				0,
				VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
			goto endCommandBuffer;
		}

		drawSceneIntoRenderTarget();
		post_process::applyPostProcessing(commandBuffer);

		/// --- In editor mode, offscreen image is used as render target
		/// --- else in game mode, swapchain image is used
#ifdef CLZ_ENABLE_EDITOR
		if (state::g_engineState == state::EngineState::Editor)
		{
			/// Don't copy render target into editor's main viewport image
			/// As most probably extents are wrong this frame
			/// Wait a frame, then draw on it
			const auto result = editor::prepareOffscreenTarget(editor::mainViewportImage);
			if (result == editor::OfffscreenPrepareResult::SAFE_TO_DRAW_ON) [[likely]]
			{
				copyPostprocessResultInto(
					editor::mainViewportImage.image,
					editor::mainViewportImage.extent,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
					VK_ACCESS_2_SHADER_READ_BIT_KHR
				);
			}
			/// else js convert its layout to shader read only
			/// and let editor read a blank image
			else
			{
				drawNullScene(
					editor::mainViewportImage.image,
					editor::mainViewportImage.imageView,
					editor::mainViewportImage.extent,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_ACCESS_2_SHADER_READ_BIT,
					VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
			}

			// ImGui chrome (panels, including the viewport image widget)
			// renders onto the swapchain image directly.
			transition_image_layout(
				r_swapchainContext.images[imageIndex],
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				0,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);

			VkRenderingAttachmentInfo colorAttachment = {};
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.pNext = nullptr;
			colorAttachment.imageView = r_swapchainContext.imageViews[imageIndex];
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = {
				.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
			};

			VkRenderingInfo renderingInfo = {};
			renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
			renderingInfo.pNext = nullptr;
			renderingInfo.flags = 0;
			renderingInfo.renderArea = {{0, 0}, r_swapchainContext.extent};
			renderingInfo.layerCount = 1;
			renderingInfo.colorAttachmentCount = 1;
			renderingInfo.pColorAttachments = &colorAttachment;
			renderingInfo.pDepthAttachment = nullptr;
			vkCmdBeginRendering(commandBuffer, &renderingInfo);

			const VkViewport viewport = {
				.x = 0.0f,
				.y = 0.0f,
				.width = static_cast<float>(r_swapchainContext.extent.width),
				.height = static_cast<float>(r_swapchainContext.extent.height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f,
			};
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			const VkRect2D scissor = {{0, 0}, r_swapchainContext.extent};
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			editor::update(commandBuffer);

			vkCmdEndRendering(commandBuffer);

			transition_image_layout(
				r_swapchainContext.images[imageIndex],
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
				0,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
				VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT,
				commandBuffer
			);

			editor::drawOffscreenTargets(commandBuffer);
		}
		else if (state::g_engineState == state::EngineState::Game)
#endif
		{
			// Post-processed result -> swapchain image directly, then present.
			copyPostprocessResultInto(
				r_swapchainContext.images[imageIndex],
				r_swapchainContext.extent,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR,
				0
			);
		}

		endCommandBuffer:
		const VkResult result = vkEndCommandBuffer(commandBuffer);
		if (result != VK_SUCCESS) [[unlikely]]
		{
			clz::log::error("vkEndCommandBuffer failed with VkResult: " + std::to_string(result));
		}

	}

	/// @copydoc
	void submitCommandBuffer(
		VkCommandBuffer commandBuffer,
		VkSemaphore renderReadySemaphore,
		VkSemaphore presentReadySemaphore,
		VkFence inFlightFence
	)
	{
		const VkSemaphoreSubmitInfoKHR waitSemaphore = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
			.pNext = nullptr,
			.semaphore = renderReadySemaphore,
			.value = 0,
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			.deviceIndex = 0
		};

		const VkSemaphoreSubmitInfoKHR signalSemaphore{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
			.pNext = nullptr,
			.semaphore = presentReadySemaphore,
			.value = 0,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR,
			.deviceIndex = 0
		};

		VkCommandBufferSubmitInfoKHR cmdSubmitInfo{};
		cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
		cmdSubmitInfo.commandBuffer = commandBuffer;

		const VkSubmitInfo2KHR submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
			.pNext = nullptr,
			.flags = 0,
			.waitSemaphoreInfoCount = 1,
			.pWaitSemaphoreInfos = &waitSemaphore,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &cmdSubmitInfo,
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos = &signalSemaphore
		};

		if (vkQueueSubmit2(r_deviceContext.graphicsQueue, 1, &submitInfo, inFlightFence) !=
		    VK_SUCCESS) [[unlikely]]
		{
			clz::log::error("renderer/mainloop: vkQueueSubmit failed");
		}
	}

	/// @copydoc
	void present(VkSemaphore semaphore, uint32_t imageIndex)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &semaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &r_swapchainContext.swapchain,
		presentInfo.pImageIndices = &imageIndex;

		const VkResult result =
			vkQueuePresentKHR(r_deviceContext.presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) [[unlikely]]
		{
			clz::log::debug("Swapchain will be recreated");
			r_swapchainOutdated = true;
		}
		else if (result != VK_SUCCESS) [[unlikely]]
		{
			clz::log::error("present failed");
		}
	}
} // namespace clz::renderer
