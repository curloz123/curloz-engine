/**
 * @file renderer.cpp
 * @author curl0z
 * @brief Implementation of the public header
 * of renderer subsystem
 */

#include "renderer/renderer.hpp"
#include "core/logs.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/context/commandcontext.hpp"
#include "renderer/context/devicecontext.hpp"
#include "renderer/context/framecontext.hpp"
#include "renderer/context/pipelinecontext.hpp"
#include "renderer/context/swapchaincontext.hpp"
#include "renderer/entitydata/entitydata.hpp"
#include "renderer/mainloop.hpp"
#include "renderer/vk_types.hpp"
#include <vector>

namespace clz::renderer
{
bool init()
{
	if (!initDeviceContext())
	{
		clz::log::error("Could not initialize device context");
		goto failure;
	}
	if (!initCommandContext())
	{
		clz::log::error("Could not initialize frame context");
		goto failure;
	}
	if (!initSwapchainContext())
	{
		clz::log::error("Could not initialize swapchain context");
		goto failure;
	}
	if (!initPipelineContexts())
	{
		clz::log::error("Could not initialize pipeline context");
		goto failure;
	}
	if (!initFrameContext())
	{
		clz::log::error("Could not initialize frame context");
		goto failure;
	}

	clz::log::info("initialized all renderer context's");
	clz::log::info("Initialized renderer");
	return true;

failure:
	clz::log::error("Could not initialize renderer");
	return false;
}

void update()
{
	if (r_recreateSwapchain) [[unlikely]]
	{
		clz::log::warn("swapchain out of date, recreating it");
		recreateSwapchainContext();

		// Update camera with current swapchain extents
		updateCameraProjMatrix(r_cameraId);
		r_recreateSwapchain = false;
	}
	waitForGPU(r_frameContext.inFlightFences[r_currentFrame]);
	acquireNextImage(r_frameContext.renderReadySemaphores[r_currentFrame], r_imageIndex);
	if (r_recreateSwapchain) [[unlikely]]
		return;
	resetFence(r_frameContext.inFlightFences[r_currentFrame]);
	startCommandBuffer(r_commandContext.commandBuffer[r_currentFrame]);

	// Everything that's not defined in mainloop.hpp, shall go inside this function
	recordCommandBuffer(r_commandContext.commandBuffer[r_currentFrame], r_imageIndex);

	submitCommandBuffer(
		r_commandContext.commandBuffer[r_currentFrame],
		r_frameContext.renderReadySemaphores[r_currentFrame],
		r_frameContext.presentReadySemaphores[r_imageIndex],
		r_frameContext.inFlightFences[r_currentFrame]
	);

	present(r_frameContext.presentReadySemaphores[r_imageIndex],
		r_imageIndex); // Internally can also r_recreateSwapchain = true

	r_currentFrame = (r_currentFrame + 1) % r_FRAMES_IN_FLIGHT;
}

void shutdown()
{
	vkDeviceWaitIdle(r_deviceContext.device);

	/// --- First Destroy Entity Data ---
	destroyEntityData();

	/// --- Destroy all context's
	destroyFrameContext();
	destroyPipelineContexts();
	destroySwapchainContext();
	destroyCommandContext();
	destroyDeviceContext();

	clz::log::info("renderer shutdown completed");
}
} // namespace clz::renderer
