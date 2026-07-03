/**
 * @file pipelinecontext.cpp
 * @author curl0z
 * @brief Implementation of the initialization
 * and cleanup of pipeline context
 */

#include "renderer/context/pipelinecontext.hpp"
#include "core/logs.hpp"
#include "renderer/shaderdata/bufferarray/uvbuffer.hpp"
#include "renderer/shaderdata/bufferarray/vertexbuffer.hpp"
#include "renderer/shaderdata/descriptor/descriptor.hpp"
#include "renderer/shaderdata/pushconstant/mainpipeline.hpp"
#include "renderer/vk_types.hpp"
#include <fstream>
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	bool initPipelineContext()
	{
		if (!createMainPipeline())
		{
			clz::log::error("Could not create main pipeline");
			clz::log::error("Could initialize pipeline context");
			return false;
		}

		clz::log::info("creating pipeline context");
		return true;
	}

	bool createShaderModules(PipelineContext& rPipelineContext, const std::string& vertexShaderLocation,
				 const std::string& fragmentShaderLocation)
	{
		// Loading Shaders
		std::ifstream vertex_file(vertexShaderLocation, std::ios::ate | std::ios::binary);
		std::ifstream frag_file(std::string(fragmentShaderLocation), std::ios::ate | std::ios::binary);
		if (!vertex_file)
		{
			clz::log::error("unable to load file: " + std::string(vertexShaderLocation));
		}
		if (!frag_file)
		{
			clz::log::error("unable to load file: " + std::string(fragmentShaderLocation));
		}

		auto vertexFileSize = vertex_file.tellg();
		std::vector<char> vertexShaderCode(vertexFileSize);
		vertex_file.seekg(0);
		vertex_file.read(vertexShaderCode.data(), (vertexFileSize));
		vertex_file.close();

		auto fragFileSize = frag_file.tellg();
		std::vector<char> fragShaderCode(fragFileSize);
		frag_file.seekg(0);
		frag_file.read(fragShaderCode.data(), fragFileSize);
		frag_file.close();

		// Creating Shader Module

		VkShaderModuleCreateInfo vertInfo = {};
		vertInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertInfo.codeSize = vertexShaderCode.size();
		vertInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderCode.data());
		if (vkCreateShaderModule(r_deviceContext.device, &vertInfo, nullptr, &r_pipelineContext.vertexShader) != VK_SUCCESS)
		{
			clz::log::error("Could not create vertex shader module" + std::string(vertexShaderLocation));

			return false;
		}

		VkShaderModuleCreateInfo fragInfo = {};
		fragInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fragInfo.codeSize = fragShaderCode.size();
		fragInfo.pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());
		if (vkCreateShaderModule(r_deviceContext.device, &fragInfo, nullptr, &r_pipelineContext.fragmentShader) != VK_SUCCESS)
		{
			clz::log::error("Could not create fragment shader module" + std::string(fragmentShaderLocation));

			return false;
		}

		clz::log::info("created shader modules: " + std::string(vertexShaderLocation) + " and " + std::string(fragmentShaderLocation));
		return true;
	}

	bool createMainPipeline()
	{
		// Create shaders modules
		auto shaderModuleResult = createShaderModules(r_pipelineContext, "shaders/triangle.vert.spirv", "shaders/triangle.frag.spirv");
		if (!shaderModuleResult) [[unlikely]]
		{
			clz::log::error("Could not create shader modules for main pipeline");
			clz::log::error("Could not create main pipeline");
			return false;
		}
		// Shader Create Info
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = r_pipelineContext.vertexShader;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = r_pipelineContext.fragmentShader;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		// Pipeline creation

		std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		std::array<VkVertexInputBindingDescription, 2> bindingDescriptions = {clz::renderer::VBuffer::getVertexBindingDescription(),
										      clz::renderer::UVBuffer::getUVBindingDescription()};
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {clz::renderer::VBuffer::getVertexAttributeDescription(),
											  clz::renderer::UVBuffer::getUVAttributeDescription()};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
									.pNext = nullptr,
									.flags = 0,
									.vertexBindingDescriptionCount = bindingDescriptions.size(),
									.pVertexBindingDescriptions = bindingDescriptions.data(),
									.vertexAttributeDescriptionCount = attributeDescriptions.size(),
									.pVertexAttributeDescriptions = attributeDescriptions.data()};

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
		// rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		// rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

		std::array<VkPushConstantRange, 2> pushConstantRange;
		pushConstantRange[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange[0].offset = 0;
		pushConstantRange[0].size = sizeof(MainPC::vertexData);
		pushConstantRange[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange[1].offset = sizeof(MainPC::vertexData);
		pushConstantRange[1].size = sizeof(MainPC::fragmentData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pushConstantRangeCount = pushConstantRange.size();
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRange.data();
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &clz::renderer::r_descriptorSetLayout;
		if (vkCreatePipelineLayout(r_deviceContext.device, &pipelineLayoutInfo, nullptr, &r_pipelineContext.layout) != VK_SUCCESS)
		    [[unlikely]]
		{
			clz::log::error("vulkan could not create pipeline layout of main pipeline");
			return false;
		}

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
		pipelineInfo.layout = r_pipelineContext.layout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(r_deviceContext.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &r_pipelineContext.pipeline) !=
		    VK_SUCCESS)
		{
			clz::log::error("vulkan Could not create pipeline");
			return false;
		}

		clz::log::info("created main pipeline");

		return true;
	}
} // namespace clz::renderer

namespace clz::renderer
{
	void destroyPipelineContext()
	{
		destroyMainPipeline();
		clz::log::info("destroyed pipeline context");
	}
	void destroyMainPipeline()
	{
		vkDestroyPipeline(r_deviceContext.device, r_pipelineContext.pipeline, nullptr);
		vkDestroyPipelineLayout(r_deviceContext.device, r_pipelineContext.layout, nullptr);

		vkDestroyShaderModule(r_deviceContext.device, r_pipelineContext.vertexShader, nullptr);
		vkDestroyShaderModule(r_deviceContext.device, r_pipelineContext.fragmentShader, nullptr);

		clz::log::info("destroyed main pipeline");
	}
} // namespace clz::renderer