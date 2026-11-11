/**
 * @file pipelines.hpp
 * @author curl0z
 * @brief Provides all backend pipeline handles and their
 * handles.
 * All pipeline's manager class is defined here, objects are created in pipelines.cpp
 */

#pragma once

#include "renderer/context/pipelinecontext.hpp"
#include "renderer/context/context.hpp"
#include "push_constants.hpp"

namespace clz::editor::backend
{
	/**
	 * @brief General pipeline interface. Each backend
	 * pipeline must publicly derive this, along with passing
	 * their push constants as T.
	 * Ps: all pipelines imo differ only by their creation,
	 * so that is virtualized :p
	 */
	template <typename T>
	class PipelineManager
	{
	protected:
		renderer::PipelineContext m_pipelineContext;

	public:
		virtual bool initialize() = 0;

		void destroy()
		{
			renderer::destroyPipelineContext(m_pipelineContext);
		}
		const renderer::PipelineContext& getPipelineContext() const
		{
			return m_pipelineContext;
		}
		void bindPipeline(VkCommandBuffer commandBuffer)
		{
			vkCmdBindPipeline(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_pipelineContext.pipeline
			);
		}
		void submitPushConstant(VkCommandBuffer commandBuffer, const T& pc)
		{
			vkCmdPushConstants(
				commandBuffer,
				m_pipelineContext.layout,
				VK_SHADER_STAGE_VERTEX_BIT |
					VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(T),
				&pc
			);
		}
	};

	///< @brief Body editor pipeline
	class BodyEditorPipeline : public PipelineManager<BodyEditorPC>
	{
	public:
		bool initialize() override;
	};
}
