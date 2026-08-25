/**
 * @file vk_types.hpp
 * @author curl0z
 * @brief Global renderer state and shared Vulkan contexts.
 *
 * This file contains globally accessible renderer state used across
 * different subsystems such as swapchain, pipeline, and frame management.
 *
 * @note These are defined as inline globals to allow header-only access.
 * All contexts are initialized during renderer setup and destroyed
 * during shutdown.
 */

#pragma once

#include "camera/camera.hpp"
#include "context/context.hpp"

namespace clz::renderer
{
	/**
	 * @brief Global device context.
	 *
	 * Holds Vulkan instance, physical device, logical device,
	 * queues, and surface.
	 */
	inline DeviceContext r_deviceContext{};


	/// @brief Commmand context of our application
	inline CommandContext r_commandContext{};

	/**
	 * @brief Global swapchain context.
	 *
	 * Contains swapchain handle, images, image views,
	 * format, extent, and presentation mode.
	 */
	inline SwapchainContext r_swapchainContext{};

	/// @brief Render target context
	inline RenderTargetContext r_renderTargetContext{};

	/**
	 * @brief Global pipeline context.
	 *
	 * Stores graphics pipeline, pipeline layout,
	 * and shader modules.
	 */
	inline PipelineContext r_pipelineContext{};

	inline PipelineContext r_bloomSamplePipelineContext{};

	/// @brief bloom post_process pipeline context
	inline PipelineContext r_bloomPipelineContext{};
	/// @brief tonemap post_process pipeline context
	inline PipelineContext r_tonemapPipelineContext{};
	/// @brief post tonemap post processes pipeline context
	inline PipelineContext r_postTonemapPipelineContext{};
	
	/**
	 * @brief Shapes pipeline context.
	 *
	 * A utility pipeline helpful to draw shapes
	 * such as cubes, cuboids, cylinder, spheres etc.
	 */
	inline PipelineContext r_shapePipelineContext{};

	/**
	 * @brief Global frame context.
	 */
	inline FrameContext r_frameContext{};

	/**
	 * @brief Index of the currently acquired swapchain image.
	 */
	inline uint32_t r_imageIndex = 0;

	/**
	 * @brief Index of the current frame in flight.
	 */
	inline uint8_t r_currentFrame = 0;

	/**
	 * @brief Number of frames processed concurrently.
	 */
	inline constexpr uint8_t r_FRAMES_IN_FLIGHT = 3;

	/// @brief Game's main camera
	inline CameraId r_cameraId = NULL_CAMERA;
} // namespace clz::renderer
