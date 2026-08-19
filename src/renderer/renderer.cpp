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
#include "renderer/context/render_target_context.hpp"
#include "renderer/postprocess/post_process.hpp"
#include <vector>

#ifdef CLZ_ENABLE_EDITOR
#include "include/editor.hpp"
#include "include/cross_system_flags.hpp"
#endif

namespace clz::renderer
{
	bool init()
	{
		/// --- 1. Initialize all context's
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

		if (!initRenderTargetContext(
			r_swapchainContext.extent.width,
			r_swapchainContext.extent.height))
		{
			clz::log::error("Could not initialize render target context");
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


		if (!post_process::initializePostProcesses())
		{
			clz::log::error("Could not initialize post process");
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
		if (r_swapchainOutdated) [[unlikely]]
		{
			clz::log::warn("swapchain out of date, recreating it");

			if (!recreateImagesOnFramebufferResize())
			{
				clz::log::error("Could not recreate images");
				return;
			}

			// Update camera with current swapchain extents
			updateCameraProjMatrix(r_cameraId);
			r_swapchainOutdated = false;
		}
		waitForGPU(r_frameContext.inFlightFences[r_currentFrame]);
		acquireNextImage(
			r_frameContext.renderReadySemaphores[r_currentFrame],
			r_imageIndex
		);
		if (r_swapchainOutdated) [[unlikely]]
			return;
		resetFence(r_frameContext.inFlightFences[r_currentFrame]);
		startCommandBuffer(r_commandContext.commandBuffer[r_currentFrame]);

		// Everything that's not defined in mainloop.hpp, shall go inside this
		// function
		recordCommandBuffer(r_commandContext.commandBuffer[r_currentFrame], r_imageIndex);

		submitCommandBuffer(
			r_commandContext.commandBuffer[r_currentFrame],
			r_frameContext.renderReadySemaphores[r_currentFrame],
			r_frameContext.presentReadySemaphores[r_imageIndex],
			r_frameContext.inFlightFences[r_currentFrame]
		);

		present(
			r_frameContext.presentReadySemaphores[r_imageIndex],
			r_imageIndex); // Internally can also do r_swapchainOutdated = true

		r_currentFrame = (r_currentFrame + 1) % r_FRAMES_IN_FLIGHT;
	}

	void shutdown()
	{
		vkDeviceWaitIdle(r_deviceContext.device);

		/// --- First Destroy Entity Data ---
		destroyEntityData();

		/// --- Then other stuff
		post_process::destroyPostProcesses();

		/// --- Destroy all context's
		destroyFrameContext();
		destroyPipelineContexts();

		destroyRenderTargetContext();
		destroySwapchainContext();
		destroyCommandContext();
		destroyDeviceContext();

		clz::log::info("renderer shutdown completed");
	}


	bool recreateImagesOnFramebufferResize()
	{
		vkDeviceWaitIdle(r_deviceContext.device);

		recreateSwapchainContext();

		if (!recreateRenderTargetContext(
			r_swapchainContext.extent.width,
			r_swapchainContext.extent.height))
		{
			clz::log::error("Could not recreate images");
			return false;
		}

		if (!post_process::recreatePostProcesses())
		{
			clz::log::error("Could not recreate images");
			return false;
		}

#ifdef CLZ_ENABLE_EDITOR
		editor::flagEditorFramebufferResize();
#endif
		return true;
	}
} // namespace clz::renderer
