/**
 * @file pipelinecontext.hpp
 * @author curl0z
 * @brief Initializes all the handles in the pipeline context
 */
#pragma once

#include "renderer/context/context.hpp"
#include <string>
#include "renderer/utility/ubo.hpp"

namespace clz::renderer
{
	/// @brief Initializes all pipeline contexts registered.
	bool initPipelineContexts();

	/**
	 * @brief Initializes shader modules for a pipeline context
	 * @param rPipelineContext Pipeline Context to create shader modules for
	 * @param vertexShaderLocation Location of vertex shader
	 * @param fragmentShaderLocation Location of fragment shader
	 * @return true if successfully created, else calls clz::error and returns false
	 */
	bool createShaderModules(PipelineContext& rPipelineContext, const std::string& vertexShaderLocation,
				 const std::string& fragmentShaderLocation);

	/**
	 * @brief Creates pipeline layout
	 * @param rPipelineContext Pipeline Context to create layout for
	 * @param pushConstantSize Size of push constant
	 * @param setLayoutCount Number of descriptor layouts
	 * @param pSetLayouts Pointer to an array of descriptor layouts
	 * @warning Supports passing only 1 push constant, that too to only vertex shader
	 */
	bool createPipelineLayout(PipelineContext& rPipelineContext, uint32_t pushConstantSize,
		uint32_t setLayoutCount, const VkDescriptorSetLayout* pSetLayouts);

	/**
	 * @brief Creates the main pipeline
	 * @return bool if succeeded
	 * else prints what error happened and continues
	 */
	bool createMainPipeline();

	/**
	 * @brief Creates the shape pipeline
	 * @return bool true is succeeded,
	 * else prints what error happened and continues
	 */
	bool createShapePipeline();
} // namespace clz::renderer

namespace clz::renderer
{
	/**
	 * @brief Renderer just have to call this once.
	 * Will automatically destroy all pipelines registered here
	 */
	void destroyPipelineContexts();
	/**
	 * @brief Destroys the pipeline context.
	 * @param pipelineContext PipelineContext object
	 * @param uboMemory Memory of uniform buffers attached
	 */
	void destroyPipelineContext(PipelineContext& pipelineContext, UBOMemory uboMemory);

} // namespace clz::renderer
