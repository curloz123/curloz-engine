
/**
 * @file drawmodel.hpp
 * @brief Rendering helpers for drawing ECS entities using the main graphics pipeline.
 */
#pragma once

#include <vulkan/vulkan.h>

namespace clz::renderer
{
	/**
	 * @brief Draws all entities containing a ModelComponent using the main pipeline.
	 *
	 * Iterates over all entities that contain both a ModelComponent and a
	 * TransformComponent, binds the required vertex/index buffers and push
	 * constants, then submits indexed draw calls for every mesh belonging
	 * to the model.
	 *
	 * @param commandBuffer Command buffer currently recording draw commands.
	 */
	void drawEntitiesMainPipeline(VkCommandBuffer commandBuffer);
} // namespace clz::renderer