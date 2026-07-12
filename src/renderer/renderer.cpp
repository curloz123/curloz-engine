/**
 * @file renderer.cpp
 * @author curl0z
 * @brief Implementation of the public header
 * of renderer subsystem
 */

#include "renderer/renderer.hpp"
#include "core/logs.hpp"
#include "renderer/assets/modeldata.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/context/commandcontext.hpp"
#include "renderer/context/devicecontext.hpp"
#include "renderer/context/framecontext.hpp"
#include "renderer/context/pipelinecontext.hpp"
#include "renderer/context/swapchaincontext.hpp"
#include "renderer/editor/editor.hpp"
#include "renderer/mainloop.hpp"
#include "renderer/shaderdata/shaderdata.hpp"
#include "renderer/vk_types.hpp"
#include <vector>

namespace clz::renderer
{
	bool init()
	{
		if (!initDeviceContext())
		{
			clz::log::error("Could not initialize device context");
			clz::log::error("Could not initialize renderer");
			return false;
		}
		if (!initCommandContext())
		{
			clz::log::error("Could not initialize frame context");
			clz::log::error("Could not initialize renderer");
			return false;
		}
		if (!initSwapchainContext())
		{
			clz::log::error("Could not initialize swapchain context");
			clz::log::error("Could not initialize renderer");
			return false;
		}
		if (!initShaderData())
		{
			clz::log::error("Could not initialize shader data");
			clz::log::error("Could not initialize renderer");
			return false;
		}
		if (!initPipelineContext())
		{
			clz::log::error("Could not initialize pipeline context");
			clz::log::error("Could not initialize renderer");
			return false;
		}
		if (!initFrameContext())
		{
			clz::log::error("Could not initialize frame context");
			clz::log::error("Could not initialize renderer");
			return false;
		}

		clz::log::info("initialized all renderer context's");

#ifdef CLZ_ENABLE_EDITOR
		if (!editor::init())
		{
			clz::log::error("Could not initialize editor");
			return false;
		}
#endif

		clz::log::info("Initialized renderer");

		return true;
	}

	void update()
	{
		// Main loop part1 - start
		if (r_recreateSwapchain) [[unlikely]]
		{
			clz::log::warn("swapchain out of date, recreating it");
			recreateSwapchainContext();
			r_recreateSwapchain = false;
		}
		waitForGPU(r_frameContext.inFlightFences[r_currentFrame]);
		acquireNextImage(r_frameContext.renderReadySemaphores[r_currentFrame], r_imageIndex);
		if (r_recreateSwapchain) [[unlikely]]
			return;
		resetFence(r_frameContext.inFlightFences[r_currentFrame]);
		startCommandBuffer(r_commandContext.commandBuffer[r_currentFrame]);
		// Main loop part1 - end

		// Main loop part2 - start
		// Everything that's not defined in mainloop.hpp, shall go inside this function
		recordCommandBuffer(r_commandContext.commandBuffer[r_currentFrame], r_imageIndex);
		// Main loop part2 - end

		// Main loop part3 - submit
		submitCommandBuffer(r_commandContext.commandBuffer[r_currentFrame], r_frameContext.renderReadySemaphores[r_currentFrame],
				    r_frameContext.presentReadySemaphores[r_imageIndex], r_frameContext.inFlightFences[r_currentFrame]);

		present(r_frameContext.presentReadySemaphores[r_imageIndex],
			r_imageIndex); // Internally can also r_recreateSwapchain = true
		// Main loop part3 - end

		r_currentFrame = (r_currentFrame + 1) % r_FRAMES_IN_FLIGHT;
	}

	void shutdown()
	{
		vkDeviceWaitIdle(r_deviceContext.device);

		editor::shutdown();

		destroyFrameContext();
		destroyPipelineContext();
		destroySwapchainContext();

		// TEST
		destroyShaderData();
		// TEST

		destroyCommandContext();
		destroyDeviceContext();

		clz::log::info("renderer shutdown completed");
	}
} // namespace clz::renderer
