#pragma once

#include "renderer/context/context.hpp"
#include <span>
#include <string>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	bool createShaderModules(PipelineContext& rPipelineContext, const std::string& vertexShaderLocation,
				 const std::string& fragmentShaderLocation);

	bool createPipelineLayout(PipelineContext& rPipelineContext, uint32_t pushConstantSize = 0, uint32_t setLayoutCount = 0,
				  const VkDescriptorSetLayout* pSetLayouts = nullptr);

	enum class ShaderStage
	{
		VERTEX,
		FRAGMENT
	};
	VkPipelineShaderStageCreateInfo createShaderStageInfo(VkShaderModule& shaderModule, ShaderStage stage);

	VkPipelineDynamicStateCreateInfo createDynamicStates(const std::vector<VkDynamicState>& dynamicStates);

	VkVertexInputBindingDescription createBindingDescription(uint32_t binding, uint32_t stride);
	VkVertexInputAttributeDescription createAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);
	VkPipelineVertexInputStateCreateInfo createVertexInputInfo(std::span<VkVertexInputBindingDescription> bindingDescriptions,
								   std::span<VkVertexInputAttributeDescription> attributeDescriptions);

	VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState(VkPrimitiveTopology topology);

	VkViewport createViewport(VkExtent2D extent);
	VkRect2D createScissor(VkExtent2D extent);
	VkPipelineViewportStateCreateInfo createViewportState(const VkViewport& viewport, const VkRect2D& scissor);

	VkPipelineRasterizationStateCreateInfo createRasterizer(VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, float lineWidth = 1.0f,
								VkCullModeFlags cullMode = VK_CULL_MODE_NONE,
								VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE);

	VkPipelineDepthStencilStateCreateInfo createDepthStencilState(bool depthEnable = true, bool stencilEnable = false);

	VkPipelineRenderingCreateInfo createPipelineRenderingInfo(uint32_t attachmentCount, const std::vector<VkFormat>& attachmentFormats,
								  VkFormat depthFormat);

} // namespace clz::renderer