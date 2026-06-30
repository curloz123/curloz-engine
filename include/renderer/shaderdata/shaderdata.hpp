/**
 * @file shaderdata.hpp
 * @author curl0z
 * @brief High level management of GPU-side shader resources.
 */
#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer
{
	/**
	 * @brief Initializes descriptor infrastructure required by shaders.
	 *
	 * Creates descriptor pools, descriptor set layouts, uniform buffers
	 * and texture subsystem state required before assets are uploaded.
	 *
	 * @return True on success, false otherwise.
	 */
	bool initShaderData();

	/**
	 * @brief Uploads all entity-related GPU resources.
	 *
	 * Submits vertex/index/UV buffers, creates textures and finalizes
	 * descriptor writes once all assets have been registered.
	 *
	 * @return True on success, false otherwise.
	 */
	bool createEntityData();

	/**
	 * @brief Updates per-frame shader resources.
	 *
	 * Updates descriptor contents and per-frame uniform data required
	 * by the current frame.
	 *
	 * @param currentFrame Current frame in flight index.
	 * @param commandBuffer Current command buffer in flight
	 */
	void updateShaderData(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	/**
	 * @brief Releases all GPU resources owned by the shader subsystem.
	 */
	void destroyShaderData();
}