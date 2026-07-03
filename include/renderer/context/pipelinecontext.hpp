/**
 * @file pipelinecontext.hpp
 * @author curl0z
 * @brief Initializes all the handles in the pipeline context
 */
#pragma once

#include "renderer/context/context.hpp"
#include <string>

namespace clz::renderer
{
	/**
	 * @brief Initializes the pipeline context.
	 * ie all the device handles.
	 * Can view all of them in context.hpp
	 */
	bool initPipelineContext();

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
	 * @brief Creates the main pipeline
	 * @return bool if succeeded
	 * else prints what error happened and continues
	 * Defined in mainpipeline.cpp
	 */
	bool createMainPipeline();
} // namespace clz::renderer

namespace clz::renderer
{
	/**
	 * @brief Destroys the pipeline context.
	 */
	void destroyPipelineContext();

	/**
	 * @brief Destroys the main pipeline
	 * Defined in mainpipeline.cpp
	 */
	void destroyMainPipeline();
} // namespace clz::renderer
