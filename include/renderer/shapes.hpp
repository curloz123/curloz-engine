#pragma once

#include "core/logs.hpp"
#include "math/mat4x4.hpp"
#include "math/vec4.hpp"
#include "renderer/vk_types.hpp"
#include "vulkan/vulkan.h"

namespace clz::renderer
{
	/// @brief  Push constants
	enum class Shape : uint32_t
	{
		BOX = 0,
	};
	struct PushConstants
	{
		math::mat4 mvp;
		math::vec4 color;
		Shape shape;
	};

	inline void drawShape(VkCommandBuffer commandBuffer, const Shape shape, const math::mat4& projection, const math::mat4& view,
			      const math::mat4& model, const math::vec4& color)
	{
		PushConstants pc = {.mvp = model * view * projection, .color = color, .shape = shape};
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_shapePipelineContext.pipeline);

		vkCmdPushConstants(commandBuffer, r_shapePipelineContext.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pc);

		switch (shape)
		{
		case Shape::BOX:
			vkCmdDraw(commandBuffer, 36, 1, 0, 0);
			break;
		default:
			clz::log::warn("Invalid shape request");
		}
	}
} // namespace clz::renderer
