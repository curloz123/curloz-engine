/**
 * @file renderer.cpp
 * @author curl0z
 * @brief Implementation of the public header
 * of renderer subsystem
 */

#include "renderer/renderer.hpp"
#include "core/logs.hpp"
#include "renderer/camera/camera.hpp"
#include "renderer/config.hpp"
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
#include "window/window.hpp"
#include <vector>

#ifdef CLZ_ENABLE_EDITOR
#include "include/editor.hpp"
#include "include/cross_system_flags.hpp"
#endif

namespace clz::renderer
{
	/// @copydoc init
	bool init()
	{
		/// --- Device is always initialized first --- ///
		if (!initDeviceContext())
		{
			clz::log::error("Could not initialize device context");
			clz::log::error("Could not initialize renderer");
		}

		/// --- Parse config data --- ///
		parseConfigData();

		/// --- Rest context's --- ///
		if (!initCommandContext())
		{
			clz::log::error("Could not initialize frame context");
			clz::log::error("Could not initialize renderer");
		}

		/// --- Get framebuffer extents --- ///
		auto [width, height] = window::getFramebufferExtents();

		if (!initSwapchainContext(width, height))
		{
			clz::log::error("Could not initialize swapchain context");
			clz::log::error("Could not initialize renderer");
		}

		if (!initRenderTargetContext(width, height))
		{
			clz::log::error("Could not initialize render target context");
			clz::log::error("Could not initialize renderer");
		}

		if (!initPipelineContexts())
		{
			clz::log::error("Could not initialize pipeline context");
			clz::log::error("Could not initialize renderer");
		}
		if (!initFrameContext())
		{
			clz::log::error("Could not initialize frame context");
			clz::log::error("Could not initialize renderer");
		}

		if (!post_process::initializePostProcesses(width, height))
		{
			clz::log::error("Could not initialize post process");
			clz::log::error("Could not initialize renderer");
		}

		clz::log::info("initialized all renderer context's");
		clz::log::info("Initialized renderer");
		return true;
	}

	/// @copydoc update
	void update()
	{
		if (r_framebufferResized) [[unlikely]]
		{
			clz::log::warn(
				"Framebuffer has been resized. "
				"Recreating outdated images"
			);

			const auto resizeResult = recreateImagesOnFramebufferResize();
			switch (resizeResult)
			{
				case ImagesResizeResult::SUCCESS:
					/// Update camera with current swapchain extents
					updateCameraProjMatrix(r_cameraId);
					r_framebufferResized = false;
					break;

				case ImagesResizeResult::INVALID_EXTENTS:
					/// Don't move further at all
					/// Just keep looping at this step
					/// Until valid extents are fetched
					return;

				case ImagesResizeResult::FAILURE:
					clz::log::error(
						"Unable to recreate images"
						"on framebuffer resize"
					);
					break;	
			}
		}

		waitForGPU(r_frameContext.inFlightFences[r_currentFrame]);
		acquireNextImage(
			r_frameContext.renderReadySemaphores[r_currentFrame],
			r_imageIndex
		);
		resetFence(r_frameContext.inFlightFences[r_currentFrame]);
		startCommandBuffer(r_commandContext.commandBuffer[r_currentFrame]);

		/// Everything that's not defined in mainloop.hpp,
		/// shall go inside this function
		recordCommandBuffer(r_commandContext.commandBuffer[r_currentFrame], r_imageIndex);

		submitCommandBuffer(
			r_commandContext.commandBuffer[r_currentFrame],
			r_frameContext.renderReadySemaphores[r_currentFrame],
			r_frameContext.presentReadySemaphores[r_imageIndex],
			r_frameContext.inFlightFences[r_currentFrame]
		);

		present(r_frameContext.presentReadySemaphores[r_imageIndex],
			r_imageIndex); // Internally can also do r_swapchainOutdated = true

		r_currentFrame = (r_currentFrame + 1) % r_FRAMES_IN_FLIGHT;
	}

	/// @copydoc shutdown
	void shutdown()
	{
		/// --- Hold up GPU!!!! wait a minute, let prv task finish first --- ///
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

		/// --- write back config data
		writeBackConfigData();

		clz::log::info("renderer shutdown completed");
	}

	/// @copydoc recreateImagesOnFramebufferResize
	ImagesResizeResult recreateImagesOnFramebufferResize()
	{
		/// --- Hold up GPU!!!! wait a minute, let prv task finish first --- ///
		vkDeviceWaitIdle(r_deviceContext.device);

		/// --- print this message always on failure
		auto printFailure = []() {
			clz::log::error("Could not recreate images :(");
		};

		/// --- Get new framebuffer extents --- ///
		auto [newWidth, newHeight] = window::getFramebufferExtents();
		if (newWidth == 0 || newHeight == 0)
		{
			clz::log::warn(
				"Either width or height of framebuffer is 0, "
				"not resizing this frame"
			);
			return ImagesResizeResult::INVALID_EXTENTS;
		}
		const uint32_t width  = static_cast<uint32_t>(newWidth);
		const uint32_t height = static_cast<uint32_t>(newHeight);

		/// --- Recreate swapchain --- ///
		if (!recreateSwapchainContext(width, height))
		{
			clz::log::error("Could not recreate swapchain context");
			printFailure();
			return ImagesResizeResult::FAILURE;
		}

		/// --- Recreate render target context --- ///
		if (!recreateRenderTargetContext(width, height))
		{
			clz::log::error("Could not recreate render target context");
			printFailure();
			return ImagesResizeResult::FAILURE;
		}

		if (!post_process::recreatePostProcesses(width, height))
		{
			clz::log::error("Could not recreate images");
			return ImagesResizeResult::FAILURE;
		}

#ifdef CLZ_ENABLE_EDITOR
		editor::flagEditorFramebufferResize(
			width,
			height
		);
#endif

		/// --- Let everything be created first --- ///
		vkDeviceWaitIdle(r_deviceContext.device);

		return ImagesResizeResult::SUCCESS;
	}


	/// @copydoc getCameraHandle
	CameraId getGameCameraHandle()
	{
		return r_cameraId;
	}
} // namespace clz::renderer
