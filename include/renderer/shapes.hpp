#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "math/vec4.hpp"
#include "math/mat4x4.hpp"
#include "vulkan/vulkan.h"
#include "renderer/vk_types.hpp"
#include "renderer/pipelineinput/shapepipeline.hpp"

namespace clz::renderer
{
	/// @brief  Push constants
	enum class Shape : uint32_t
	{
		BOX = 0,
	};
	struct PushConstants
	{
		math::mat4 model;
		math::vec4 color;
		Shape shape;
	};

	inline void drawShape(VkCommandBuffer commandBuffer, const Shape shape,
			const math::mat4& projection, const math::mat4& view,
			const math::mat4& model, const math::vec4& color)
	{
		PushConstants pc = {
			.model = model,
			.color = color,
			.shape = shape
		};
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_shapePipelineContext.pipeline);
		ShapePipeline::updateCameraUBO(projection, view);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_shapePipelineContext.layout, 0, 1,
					&r_shapePipelineContext.descriptorSets[r_currentFrame], 0, nullptr);

		vkCmdPushConstants(commandBuffer, r_shapePipelineContext.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pc);

		switch(shape)
		{
		case Shape::BOX:
			vkCmdDraw(commandBuffer, 36, 1, 0, 0);
			break;
		default:
			clz::log::warn("Invalid shape request");
		}


	}
}
