/**
 * @file mainloop.cpp
 * @author curl0z
 * @brief Implementation of the main loop of the frame
 */

#include "renderer/mainloop.hpp"
#include "core/enginestate.hpp"
#include "core/logs.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/drawscene.hpp"
#include "renderer/model/model.hpp"
#include "renderer/renderer.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/vk_types.hpp"
#include "window/mouse.hpp"

#ifdef CLZ_ENABLE_EDITOR
#include "include/editor.hpp"
#include "include/sceneview.hpp"
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
		r_recreateSwapchain = true;
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

/// @copydoc
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	/// --- In editor mode, offscreen image is usef as render target
	/// --- else in game mode, swapchain image is used
#ifdef CLZ_ENABLE_EDITOR
	if (state::g_engineState == state::EngineState::Editor)
	{
		editor::prepareOffscreenTarget(editor::mainViewportImage);
		transition_image_layout(
			editor::mainViewportImage.image,
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
		colorAttachment.imageView = editor::mainViewportImage.imageView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {
			.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
		};

		VkRenderingAttachmentInfoKHR depthAttachment = {};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthAttachment.imageView = editor::mainViewportImage.depthImageView;
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.clearValue.depthStencil.depth = 1.0f;

		VkRenderingInfoKHR renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderingInfo.pNext = nullptr;
		renderingInfo.flags = 0;
		renderingInfo.renderArea = {{0, 0}, editor::mainViewportImage.extent};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = &depthAttachment;

		vkCmdBeginRendering(commandBuffer, &renderingInfo);
		VkViewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(editor::mainViewportImage.extent.width),
			.height =
				static_cast<float>(editor::mainViewportImage.extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{{0, 0}, editor::mainViewportImage.extent};
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		/// --- Main artist ---
		drawScene(commandBuffer);
		/// --- artist finished drawing ---

		vkCmdEndRendering(commandBuffer);
		transition_image_layout(
			editor::mainViewportImage.image,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			0,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR,
			VK_IMAGE_ASPECT_COLOR_BIT,
			commandBuffer
		);

		transition_image_layout(
			r_swapchainContext.images[imageIndex],
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			0,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
			VK_IMAGE_ASPECT_COLOR_BIT,
			commandBuffer
		);

		colorAttachment = {};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachment.pNext = nullptr;
		colorAttachment.imageView = r_swapchainContext.imageViews[imageIndex];
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {
			.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
		};

		depthAttachment = {};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthAttachment.imageView = r_swapchainContext.depthImageView;
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.clearValue.depthStencil.depth = 1.0f;

		renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderingInfo.pNext = nullptr;
		renderingInfo.flags = 0;
		renderingInfo.renderArea = {{0, 0}, r_swapchainContext.extent};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = &depthAttachment;
		vkCmdBeginRendering(commandBuffer, &renderingInfo);

		viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(r_swapchainContext.extent.width),
			.height = static_cast<float>(r_swapchainContext.extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		scissor = {{0, 0}, r_swapchainContext.extent};
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
		transition_image_layout(
			r_swapchainContext.images[imageIndex],
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
		colorAttachment.imageView = r_swapchainContext.imageViews[imageIndex];
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = {
			.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
		};

		VkRenderingAttachmentInfo depthAttachment = {};
		depthAttachment = {};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthAttachment.imageView = r_swapchainContext.depthImageView;
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.clearValue.depthStencil.depth = 1.0f;

		VkRenderingInfo renderingInfo = {};
		renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderingInfo.pNext = nullptr;
		renderingInfo.flags = 0;
		renderingInfo.renderArea = {{0, 0}, r_swapchainContext.extent};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = &depthAttachment;
		vkCmdBeginRendering(commandBuffer, &renderingInfo);

		VkViewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(r_swapchainContext.extent.width),
			.height = static_cast<float>(r_swapchainContext.extent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor = {{0, 0}, r_swapchainContext.extent};
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		/// --- Main artist ---
		drawScene(commandBuffer);
		/// --- artist finished drawing ---

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
	}

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) [[unlikely]]
	{
		clz::log::error("renderer/mainloop: vkEndCommandBuffer failed");
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
		r_recreateSwapchain = true;
	}
	else if (result != VK_SUCCESS) [[unlikely]]
	{
		clz::log::error("present failed");
	}
}
} // namespace clz::renderer
