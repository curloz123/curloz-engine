/**
 * @file backend.hpp
 * @author curl0z
 * @brief Has all vulkan context's required for offscreen stuff
 * of that particular pipeline.
 */

#pragma once

#include "pipelines.hpp"

namespace clz::editor::backend
{
	/// Body editor pipeline
	inline BodyEditorPipeline bodyEditorPipeline;


	/// @brief Initializes all pipeline's, UBO's, SSBO's etc.. pipelinedata.
	/// @return true on successful creation, false otherwise.
	bool init();

	/// @brief Shuts down backend ie. destroys all pipeline data.
	void shutdown();

} // namespace clz::editor::backend
