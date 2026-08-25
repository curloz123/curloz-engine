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
#include "renderer/pipelinedata/post_process.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"
#include "renderer/pipelinedata/texture.hpp"
#include "renderer/shapes.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/pipeline.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/postprocess/bloom_sample.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/postprocess/tonemap.hpp"
#include "renderer/postprocess/bloom.hpp"
#include <vector>

namespace clz::renderer
{
	static bool createBloomSamplePipeline();
	static bool createBloomPipeline();
	static bool createTonemapPipeline();
	static bool createPost_TonemapPipeline();
}
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

		if (!createBloomPipeline())
		{
			clz::log::error("Could not create bloom pipeline");
			clz::log::error("Could initialize pipeline context");
			return false;

		}

		if (!createBloomSamplePipeline())
		{
			clz::log::error("Could not create bloom sampling pipeline");
			clz::log::error("Could initialize pipeline context");
			return false;
		}

		if (!createTonemapPipeline())
		{
			clz::log::error("Could not create tonemap pipeline");
			clz::log::error("Could not initialize pipeline context");
			return false;
		}

		if (!createPost_TonemapPipeline())
		{
			clz::log::error("Could not create post tonemap pipeline");
			clz::log::error("Could not initialize pipeline context");
			return false;
		}

		clz::log::info("created pipeline context");
		return true;
	}
}

namespace clz::renderer
{
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
			getVertexBindingDescription()
		};
		std::array attributeDescriptions = {
			getVertexAttributeDescription(VertexAttributeType::POSITION),
			getVertexAttributeDescription(VertexAttributeType::UV),
			getVertexAttributeDescription(VertexAttributeType::NORMAL),
			getVertexAttributeDescription(VertexAttributeType::TANGENT),
		};
		VkPipelineVertexInputStateCreateInfo vertexInputInfo =
			createVertexInputInfo(bindingDescriptions, attributeDescriptions);

		VkPipelineInputAssemblyStateCreateInfo inputAssembly =
			createInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		VkViewport viewport = createViewport(r_renderTargetContext.imageExtent);
		VkRect2D scissor = createScissor(r_renderTargetContext.imageExtent);
		VkPipelineViewportStateCreateInfo viewportState =
			createViewportState(viewport, scissor);

		VkPipelineRasterizationStateCreateInfo rasterizer =
			createRasterizer(VK_POLYGON_MODE_FILL, 1.0f);

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.rasterizationSamples = r_renderTargetContext.msaaFlagBits;

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

		std::vector<VkFormat> attachmentFormats = {r_renderTargetContext.imageFormat};
		VkPipelineRenderingCreateInfo pipelineRenderingCI = createPipelineRenderingInfo(
			1,
			attachmentFormats,
			r_renderTargetContext.depthFormat
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
		viewport.width = static_cast<float>(r_renderTargetContext.imageExtent.width);
		viewport.height = static_cast<float>(r_renderTargetContext.imageExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = r_renderTargetContext.imageExtent;

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
		multisampling.rasterizationSamples = r_renderTargetContext.msaaFlagBits;

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
		pipelineRenderingCI.pColorAttachmentFormats = &r_renderTargetContext.imageFormat;
		pipelineRenderingCI.depthAttachmentFormat = r_renderTargetContext.depthFormat;

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

	/// @brief Shared setup for the full-screen post-process pipelines
	/// (pre/post-tonemap and tonemap). Both are full-screen-triangle passes:
	/// no vertex buffer (position/UV come from gl_VertexIndex in the vertex
	/// shader), no depth testing, single color attachment, and bind against
	/// the shared post-process descriptor set layout.
	static bool createFullScreenPostProcessPipeline(
		PipelineContext& pipelineContext,
		const char* vertPath,
		const char* fragPath,
		const char* debugName,
		const size_t pushConstantSize,
		VkFormat colorAttachmentFormat,
		VkExtent2D extent)
	{
		if (!createShaderModules(pipelineContext, vertPath, fragPath))
		{
			clz::log::error(std::string("Could not create shader modules for ") + debugName);
			return false;
		}

		auto vertShaderStageInfo = createShaderStageInfo(
			pipelineContext.vertexShader,
			ShaderStage::VERTEX);
		auto fragShaderStageInfo = createShaderStageInfo(
			pipelineContext.fragmentShader,
			ShaderStage::FRAGMENT
		);
		std::array shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

		// Dynamic State
		std::vector dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicState = createDynamicStates(dynamicStates);

		// Full-screen triangle: no vertex buffer bound, ever. Both counts
		// MUST be zero -- position/UV are derived from gl_VertexIndex inside
		// post_process.vert, matching the vkCmdDraw(cmd, 3, 1, 0, 0) call
		// used to invoke this pipeline.
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly =
			createInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		VkViewport viewport = createViewport(extent);
		VkRect2D scissor = createScissor(extent);
		VkPipelineViewportStateCreateInfo viewportState =
			createViewportState(viewport, scissor);

		VkPipelineRasterizationStateCreateInfo rasterizer =
			createRasterizer(VK_POLYGON_MODE_FILL, 1.0f);

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// No depth: flat full-screen pass, nothing to test/write against.
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
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
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		// Post-process passes sample the previous stage's image through the
		// shared post-process descriptor set layout (pre/tonemap/post bind points).
		std::vector<VkDescriptorSetLayout> layouts = {
			post_processDescriptorLayout
		};
		createPipelineLayout(
			pipelineContext,
			pushConstantSize,
			layouts.size(),
			layouts.data()
		);

		std::vector<VkFormat> attachmentFormats = {colorAttachmentFormat};
		// No depth attachment for a post-process pass.
		VkPipelineRenderingCreateInfo pipelineRenderingCI = createPipelineRenderingInfo(
			1,
			attachmentFormats,
			VK_FORMAT_UNDEFINED
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
		pipelineInfo.layout = pipelineContext.layout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(
			    r_deviceContext.device,
			    VK_NULL_HANDLE,
			    1,
			    &pipelineInfo,
			    nullptr,
			    &pipelineContext.pipeline
		    ) != VK_SUCCESS)
		{
			clz::log::error(std::string("vulkan could not create ") + debugName);
			return false;
		}

		clz::log::info(std::string("created ") + debugName);

		setHandleName(
			reinterpret_cast<uint64_t>(pipelineContext.pipeline),
			VK_OBJECT_TYPE_PIPELINE,
			debugName
		);

		return true;
	}

	static bool createBloomSamplePipeline()
	{
		// One pipeline handles both pre- and post-tonemap passes; which
		// mode runs is selected via Pre_PostTonemapPC.mode at draw time.
		// Color attachment format matches pre_tonemapImage / post_tonemapImage,
		// both currently created with r_renderTargetContext.imageFormat.
		// NOTE: if post_tonemapImage's format is changed to a dedicated LDR
		// format later (flagged in post_tonemap.cpp), this single pipeline
		// will need to be split into two, since VkPipelineRenderingCreateInfo's
		// color format must match the attachment bound at draw time.
		return createFullScreenPostProcessPipeline(
			r_bloomSamplePipelineContext,
			"shaders/post_process.vert.spirv",
			"shaders/bloom_sample.frag.spirv",
			"pre tonemap pipeline",
			0,
			post_process::BLOOM_SAMPLE_IMAGE_FORMAT,
			r_renderTargetContext.imageExtent
		);
	}

	static bool createBloomPipeline()
	{
		return createFullScreenPostProcessPipeline(
			r_bloomPipelineContext,
			"shaders/post_process.vert.spirv",
			"shaders/bloom.frag.spirv",
			"bloom pipeline",
			sizeof(BloomPC),
			post_process::BLOOM_IMAGE_FORMAT,
			r_renderTargetContext.imageExtent
		);
	}

	static bool createTonemapPipeline()
	{
		return createFullScreenPostProcessPipeline(
			r_tonemapPipelineContext,
			"shaders/post_process.vert.spirv",
			"shaders/tonemap.frag.spirv",
			"tonemap pipeline",
			sizeof(TonemapPC),
			post_process::TONEMAP_IMAGE_FORMAT,
			r_renderTargetContext.imageExtent
		);
	}

	static bool createPost_TonemapPipeline()
	{
		return createFullScreenPostProcessPipeline(
			r_postTonemapPipelineContext,
			"shaders/post_process.vert.spirv",
			"shaders/post_tonemap.frag.spirv",
			"post tonemap pipeline",
			sizeof(Post_TonemapPC),
			post_process::POST_TONEMAP_IMAGE_FORMAT,
			r_swapchainContext.extent
		);
	}

} // namespace clz::renderer

namespace clz::renderer
{
	void destroyPipelineContexts()
	{
		// --- Destroy pipeline data first
		destroyPipelineData();

		// --- Then Destroy pipeline context's
		destroyPipelineContext(r_postTonemapPipelineContext);
		clz::log::info("destroyed post tonemap pipeline context");
		destroyPipelineContext(r_tonemapPipelineContext);
		clz::log::info("destroyed tonemap pipeline context");
		destroyPipelineContext(r_bloomPipelineContext);
		clz::log::info("destroyed bloom pipeline context");
		destroyPipelineContext(r_bloomSamplePipelineContext);
		clz::log::info("destroyed bloom sampling pipeline context");
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
