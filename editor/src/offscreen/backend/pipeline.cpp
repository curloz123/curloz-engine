#include "../../../include/offscreen/backend/pipeline.hpp"
#include "renderer/entitydata/uvbuffer.hpp"
#include "renderer/utility/descriptor.hpp"
#include "core/logs.hpp"
#include "renderer/context/pipelinecontext.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "renderer/vk_types.hpp"

namespace clz::editor::backend
{
	bool initializeEditorPipeline()
	{
		// Pipeline creation
		if (!renderer::createShaderModules(editorPipelineContext,
				"shaders/editor.vert.spirv", "shaders/editor.frag.spirv"))
		{
			clz::log::error("could not create editor shaders");
			return false;
		}

		createDescriptorSetLayout();
		std::vector<renderer::UBO*> pUniformBuffers = {&CameraUBO};
		renderer::createDescriptor(editorPipelineContext, true,
					pUniformBuffers, uboMemory, true);
		if (!renderer::createPipelineLayout(editorPipelineContext,
				sizeof(renderer::ModelDataPC), 1, &editorPipelineContext.descriptorSetLayout))
		{
			clz::log::error("could not create editor pipeline layout");
			return false;
		}

		// Shader Create Info
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = editorPipelineContext.vertexShader;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = editorPipelineContext.fragmentShader;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


		std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		std::array<VkVertexInputBindingDescription, 2> bindingDescriptions = {clz::renderer::VBuffer::getVertexBindingDescription(),
										      clz::renderer::UVBuffer::getUVBindingDescription()};
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {clz::renderer::VBuffer::getVertexAttributeDescription(),
											  clz::renderer::UVBuffer::getUVAttributeDescription()};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.vertexBindingDescriptionCount = bindingDescriptions.size(),
			.pVertexBindingDescriptions = bindingDescriptions.data(),
			.vertexAttributeDescriptionCount = attributeDescriptions.size(),
			.pVertexAttributeDescriptions = attributeDescriptions.data()
		};

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(renderer::r_swapchainContext.extent.width);
		viewport.height = static_cast<float>(renderer::r_swapchainContext.extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = renderer::r_swapchainContext.extent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		//rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;


		VkPipelineRenderingCreateInfo pipelineRenderingCI = {};
		pipelineRenderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingCI.colorAttachmentCount = 1;
		pipelineRenderingCI.pColorAttachmentFormats = &renderer::r_swapchainContext.format.format;
		pipelineRenderingCI.depthAttachmentFormat = renderer::r_swapchainContext.depthFormat;

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
		pipelineInfo.layout = editorPipelineContext.layout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(renderer::r_deviceContext.device, VK_NULL_HANDLE,
			1, &pipelineInfo, nullptr, &editorPipelineContext.pipeline) != VK_SUCCESS)
		{
			clz::log::error("vulkan Could not create editor's pipeline");
			return false;
		}

		clz::log::info("created editor's pipeline");
		return true;

	}

	void destroyEditorPipeline()
	{
		renderer::destroyUniformBuffer(uboMemory);
		renderer::destroyPipelineContext(editorPipelineContext, uboMemory);
		clz::log::info("destroyed editor's pipeline");
	}

	void updateTextureBufferObject()
	{
		renderer::updateTextureData(editorPipelineContext);
	}
}