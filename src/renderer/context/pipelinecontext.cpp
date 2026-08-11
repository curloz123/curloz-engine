/**
 * @file pipelinecontext.cpp
 * @author curl0z
 * @brief Implementation of the initialization
 * and cleanup of pipeline context
 * InitPipelineContexts initializes all the pipelines
 * used in main rendering only. Editor's pipeline must
 * be managed seperately.
 *
 * @note Before creating any pipeline context,
 * make sure to run "createPipelineData".
 * It prepares all the descriptor layouts sets ubo's etc...
 */

#include "renderer/context/pipelinecontext.hpp"
#include "core/logs.hpp"
#include "renderer/context/context.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "renderer/pipelinedata/camera.hpp"
#include "renderer/pipelinedata/lights.hpp"
#include "renderer/pipelinedata/pipelinedata.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"
#include "renderer/pipelinedata/texture.hpp"
#include "renderer/shapes.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/pipeline.hpp"
#include "renderer/vk_types.hpp"

#include <vector>

namespace clz::renderer
{
	bool initPipelineContexts()
	{
		// --- Initialize pipeline data ---
		preparePipelineData();

		// --- Now create all pipelines

		/// --- Main pipeline ---
		if (!createMainPipeline())
		{
			clz::log::error("Could not create main pipeline");
			clz::log::error("Could initialize pipeline context");
			return false;
		}

		/// --- Shape pipeline ---
		if (!createShapePipeline())
		{
			clz::log::error("Could not create shape pipeline");
			clz::log::error("Could initialize pipeline context");
			return false;
		}

		clz::log::info("created pipeline context");
		return true;
	}

	bool createMainPipeline()
	{
		// Create shaders modules
		if (!createShaderModules(
			    r_pipelineContext,
			    "shaders/mainpipeline.vert.spirv",
			    "shaders/mainpipeline.frag.spirv"
		    ))
		{
			clz::log::error("Could not create shader modules for main pipeline");
			clz::log::error("Could not create main pipeline");
			return false;
		}
		// Shader Create Info
		auto vertShaderStageInfo =
			createShaderStageInfo(r_pipelineContext.vertexShader, ShaderStage::VERTEX);
		auto fragShaderStageInfo = createShaderStageInfo(
			r_pipelineContext.fragmentShader,
			ShaderStage::FRAGMENT
		);
		std::array shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

		// Dynamic State
		std::vector dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicState = createDynamicStates(dynamicStates);

		std::array bindingDescriptions = {
			getVertexBindingDescription(),
		};
		std::array attributeDescriptions = {
			getVertexAttributeDescription(VertexAttributeType::POSITION),
			getVertexAttributeDescription(VertexAttributeType::UV),
			getVertexAttributeDescription(VertexAttributeType::NORMAL),
		};
		VkPipelineVertexInputStateCreateInfo vertexInputInfo =
			createVertexInputInfo(bindingDescriptions, attributeDescriptions);

		VkPipelineInputAssemblyStateCreateInfo inputAssembly =
			createInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		VkViewport viewport = createViewport(r_swapchainContext.extent);
		VkRect2D scissor = createScissor(r_swapchainContext.extent);
		VkPipelineViewportStateCreateInfo viewportState =
			createViewportState(viewport, scissor);

		VkPipelineRasterizationStateCreateInfo rasterizer =
			createRasterizer(VK_POLYGON_MODE_FILL, 1.0f);

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil =
			createDepthStencilState(true, false);

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

		std::vector<VkDescriptorSetLayout> layouts = {
			cameraDescriptorLayout,
			textureDescriptorLayout,
			lightDescriptorLayout
		};
		size_t pushConstantSize = sizeof(ModelDataPC);
		createPipelineLayout(
			r_pipelineContext,
			pushConstantSize,
			layouts.size(),
			layouts.data()
		);

		std::vector<VkFormat> attachmentFormats = {r_swapchainContext.format.format};
		VkPipelineRenderingCreateInfo pipelineRenderingCI = createPipelineRenderingInfo(
			1,
			attachmentFormats,
			r_swapchainContext.depthFormat
		);

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = &pipelineRenderingCI;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = r_pipelineContext.layout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(
			    r_deviceContext.device,
			    VK_NULL_HANDLE,
			    1,
			    &pipelineInfo,
			    nullptr,
			    &r_pipelineContext.pipeline
		    ) != VK_SUCCESS)
		{
			clz::log::error("vulkan Could not create shape pipeline");
			return false;
		}

		clz::log::info("created main pipeline");

		setHandleName(
			reinterpret_cast<uint64_t>(r_pipelineContext.pipeline),
			VK_OBJECT_TYPE_PIPELINE,
			"main pipeline"
		);

		return true;
	}

	bool createShapePipeline()
	{
		// Create shaders modules
		if (!createShaderModules(
			    r_shapePipelineContext,
			    "shaders/shapes.vert.spirv",
			    "shaders/shapes.frag.spirv"
		    ))
		{
			log::error("Could not create pipeline");
			return false;
		}
		// Shader Create Info
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = r_shapePipelineContext.vertexShader;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = r_shapePipelineContext.fragmentShader;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertShaderStageInfo,
			fragShaderStageInfo
		};

		// Pipeline creation

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr
		};

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(r_swapchainContext.extent.width);
		viewport.height = static_cast<float>(r_swapchainContext.extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = r_swapchainContext.extent;

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

		createPipelineLayout(
			r_shapePipelineContext,
			sizeof(ShapePushConstants),
			0,
			nullptr
		);

		VkPipelineRenderingCreateInfo pipelineRenderingCI = {};
		pipelineRenderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingCI.colorAttachmentCount = 1;
		pipelineRenderingCI.pColorAttachmentFormats = &r_swapchainContext.format.format;
		pipelineRenderingCI.depthAttachmentFormat = r_swapchainContext.depthFormat;

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
		pipelineInfo.layout = r_shapePipelineContext.layout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(
			    r_deviceContext.device,
			    VK_NULL_HANDLE,
			    1,
			    &pipelineInfo,
			    nullptr,
			    &r_shapePipelineContext.pipeline
		    ) != VK_SUCCESS)
		{
			clz::log::error("vulkan Could not create shape pipeline");
			return false;
		}

		clz::log::info("created shape pipeline");

		setHandleName(
			reinterpret_cast<uint64_t>(r_shapePipelineContext.pipeline),
			VK_OBJECT_TYPE_PIPELINE,
			"shape pipeline"
		);

		return true;
	}
} // namespace clz::renderer

namespace clz::renderer
{
	void destroyPipelineContexts()
	{
		// --- Destroy pipeline data first
		destroyPipelineData();

		// --- Then Destroy pipeline context's
		destroyPipelineContext(r_shapePipelineContext);
		clz::log::info("destroyed main pipeline context");
		destroyPipelineContext(r_pipelineContext);
		clz::log::info("destroyed shape pipeline context");
	}

	void destroyPipelineContext(PipelineContext& pipelineContext)
	{
		vkDestroyPipeline(r_deviceContext.device, pipelineContext.pipeline, nullptr);
		vkDestroyPipelineLayout(r_deviceContext.device, pipelineContext.layout, nullptr);
		vkDestroyShaderModule(
			r_deviceContext.device,
			pipelineContext.vertexShader,
			nullptr
		);
		vkDestroyShaderModule(
			r_deviceContext.device,
			pipelineContext.fragmentShader,
			nullptr
		);
	}

} // namespace clz::renderer
