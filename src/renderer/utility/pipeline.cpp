#include "renderer/utility/pipeline.hpp"

#include "core/assert.hpp"
#include "core/logs.hpp"
#include "renderer/vk_types.hpp"
#include <fstream>

namespace clz::renderer
{
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
		if (vkCreateShaderModule(r_deviceContext.device, &vertInfo, nullptr, &rPipelineContext.vertexShader) != VK_SUCCESS)
		{
			clz::log::error("Could not create vertex shader module" + std::string(vertexShaderLocation));

			return false;
		}

		VkShaderModuleCreateInfo fragInfo = {};
		fragInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fragInfo.codeSize = fragShaderCode.size();
		fragInfo.pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());
		if (vkCreateShaderModule(r_deviceContext.device, &fragInfo, nullptr, &rPipelineContext.fragmentShader) != VK_SUCCESS)
		{
			clz::log::error("Could not create fragment shader module" + std::string(fragmentShaderLocation));

			return false;
		}

		clz::log::info("created shader modules: " + std::string(vertexShaderLocation) + " and " + std::string(fragmentShaderLocation));
		return true;
	}

	bool createPipelineLayout(PipelineContext& rPipelineContext, const uint32_t pushConstantSize, const uint32_t setLayoutCount,
				  const VkDescriptorSetLayout* pSetLayouts)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		VkPushConstantRange pushConstantRange = {};
		if (pushConstantSize != 0)
		{
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = pushConstantSize;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		}
		else
		{
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;
		}

		pipelineLayoutInfo.setLayoutCount = setLayoutCount;
		pipelineLayoutInfo.pSetLayouts = pSetLayouts;
		if (vkCreatePipelineLayout(r_deviceContext.device, &pipelineLayoutInfo, nullptr, &rPipelineContext.layout) != VK_SUCCESS) [[unlikely]]
		{
			clz::log::error("vulkan could not create pipeline layout of main pipeline");
			return false;
		}

		return true;
	}

	VkPipelineShaderStageCreateInfo createShaderStageInfo(VkShaderModule& shaderModule, ShaderStage stage)
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		if (stage == ShaderStage::VERTEX)
			shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		else if (stage == ShaderStage::FRAGMENT)
			shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";

		return shaderStageInfo;
	}

	VkPipelineDynamicStateCreateInfo createDynamicStates(const std::vector<VkDynamicState>& dynamicStates)
	{
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = dynamicStates.size();
		dynamicState.pDynamicStates = dynamicStates.data();

		return dynamicState;
	}

	VkVertexInputBindingDescription createBindingDescription(uint32_t binding, uint32_t stride)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = binding;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	VkVertexInputAttributeDescription createAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = binding;
		attributeDescription.location = location;
		attributeDescription.format = format;
		attributeDescription.offset = offset;

		return attributeDescription;
	}
	VkPipelineVertexInputStateCreateInfo createVertexInputInfo(std::span<VkVertexInputBindingDescription> bindingDescriptions,
								   std::span<VkVertexInputAttributeDescription> attributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		    .pNext = nullptr,
		    .flags = 0,
		    .vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
		    .pVertexBindingDescriptions = bindingDescriptions.data(),
		    .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		    .pVertexAttributeDescriptions = attributeDescriptions.data()};

		return vertexInputInfo;
	}

	VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState(VkPrimitiveTopology topology)
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = topology;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		return inputAssembly;
	}

	VkViewport createViewport(const VkExtent2D extent)
	{
		return VkViewport{.x = 0.0f,
				  .y = 0.0f,
				  .width = static_cast<float>(extent.width),
				  .height = static_cast<float>(extent.height),
				  .minDepth = 0.0f,
				  .maxDepth = 1.0f};
	}
	VkRect2D createScissor(const VkExtent2D extent)
	{
		return VkRect2D{.offset = {0, 0}, .extent = extent};
	}

	VkPipelineViewportStateCreateInfo createViewportState(const VkViewport& viewport, const VkRect2D& scissor)
	{

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// FIX THIS
		return viewportState;
	}

	VkPipelineRasterizationStateCreateInfo createRasterizer(VkPolygonMode polygonMode, float lineWidth, VkCullModeFlags cullMode,
								VkFrontFace frontFace)
	{
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.lineWidth = lineWidth;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.polygonMode = polygonMode;
		rasterizer.cullMode = cullMode;
		if (cullMode == VK_CULL_MODE_NONE)
		{
			return rasterizer;
		}

		rasterizer.frontFace = frontFace;
		return rasterizer;
	}

	VkPipelineDepthStencilStateCreateInfo createDepthStencilState(const bool depthEnable, const bool stencilEnable)
	{
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = (depthEnable) ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = (depthEnable) ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = (stencilEnable) ? VK_TRUE : VK_FALSE;

		return depthStencil;
	}

	VkPipelineRenderingCreateInfo createPipelineRenderingInfo(const uint32_t attachmentCount, const std::vector<VkFormat>& attachmentFormats,
								  const VkFormat depthFormat)
	{
		clz::CLZ_ASSERT(attachmentFormats.size() == attachmentCount, "attachmentFormats size and attachmentCount must be the same");

		VkPipelineRenderingCreateInfo pipelineRenderingCI = {};
		pipelineRenderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingCI.colorAttachmentCount = attachmentCount;
		pipelineRenderingCI.pColorAttachmentFormats = attachmentFormats.data();
		pipelineRenderingCI.depthAttachmentFormat = depthFormat;

		return pipelineRenderingCI;
	}
} // namespace clz::renderer