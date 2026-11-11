/**
 * @file pipelines.cpp
 * @author curl0z
 * @brief Defines all pipelines creation etc...
 */

#include "../../../include/offscreen/backend/pipelines.hpp"
#include "../../../include/offscreen/backend/push_constants.hpp"
#include "../../../include/offscreen/offscreentarget.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/utility/pipeline.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/entitydata/buffer.hpp"
#include "renderer/pipelinedata/pipeline_data.hpp"

namespace clz::editor::backend
{
	bool BodyEditorPipeline::initialize()
	{
		if (!renderer::createShaderModules(
			m_pipelineContext,
			"shaders/body_editor.vert.spirv",
			"shaders/body_editor.frag.spirv"
		))
		{
			clz::log::error(
				"Could not create shape editor pipeline",
				true
			);
			return false;
		}

		// Shader Create Info
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = m_pipelineContext.vertexShader;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = m_pipelineContext.fragmentShader;
		fragShaderStageInfo.pName = "main";

		const VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertShaderStageInfo,
			fragShaderStageInfo
		};

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		std::array bindingDescriptions = {
			renderer::getStaticVertexBindingDescription()
		};
		std::array attributeDescriptions = {
			getStaticVertexAttributeDescription(renderer::VertexAttributeType::POSITION),
			getStaticVertexAttributeDescription(renderer::VertexAttributeType::UV)
		};
		const VkPipelineVertexInputStateCreateInfo vertexInputInfo =
			renderer::createVertexInputInfo(
				bindingDescriptions,
				attributeDescriptions
			);

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		std::vector<VkDescriptorSetLayout> layouts(
			renderer::NUM_DESCRIPTORS,
			renderer::getEmptyLayout()
		);
		renderer::TextureDescriptor.populateLayoutArray(layouts);
		size_t pushConstantSize = sizeof(BodyEditorPC);
		createPipelineLayout(
			m_pipelineContext,
			pushConstantSize,
			layouts.size(),
			layouts.data()
		);

		VkPipelineRenderingCreateInfo pipelineRenderingCI = {};
		pipelineRenderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingCI.colorAttachmentCount = 1;
		pipelineRenderingCI.pColorAttachmentFormats = &renderer::r_renderTargetContext.imageFormat;
		pipelineRenderingCI.depthAttachmentFormat = renderer::r_renderTargetContext.depthFormat;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = &pipelineRenderingCI;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_pipelineContext.layout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(
			    renderer::r_deviceContext.device,
			    VK_NULL_HANDLE,
			    1,
			    &pipelineInfo,
			    nullptr,
			    &m_pipelineContext.pipeline
		    ) != VK_SUCCESS)
		{
			clz::log::error(
				"vulkan Could not create body-editor pipeline",
				true
			);
			return false;
		}

		clz::log::info("created shape-editor pipeline");

		renderer::setHandleName(
			reinterpret_cast<uint64_t>(m_pipelineContext.pipeline),
			VK_OBJECT_TYPE_PIPELINE,
			"shape-editor pipeline"
		);

		return true;
	}
}
