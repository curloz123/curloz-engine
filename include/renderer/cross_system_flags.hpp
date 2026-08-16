#pragma once

#include "renderer.hpp"
#include "vk_types.hpp"
#include "context/render_target_context.hpp"
#include "context/swapchaincontext.hpp"
#include "renderer/camera/camera.hpp"
#include "core/enginestate.hpp"
#include "pipelinedata/post_process.hpp"
#include "postprocess/post_process.hpp"

namespace clz::renderer
{
	inline void flagRendererStateChange(
		const clz::state::EngineState oldState,
		const clz::state::EngineState newState)
	{
		clz::log::debug("state change called by renderer");
		if (newState == clz::state::EngineState::Game)
		{
			window::disableCursor();
		}
	}

#ifdef CLZ_ENABLE_EDITOR
	inline void flagRendererEditorInitialized()
	{
		updatePostProcessDescriptorSets();
	}
#endif

	inline void flagRendererFramebufferResize()
	{
		recreateImagesOnFramebufferResize();
	}
}