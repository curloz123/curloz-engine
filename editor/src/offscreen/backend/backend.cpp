#include "../../../include/offscreen/backend/backend.hpp"
#include "../../../include/offscreen/backend/pipeline.hpp"
#include "core/logs.hpp"

namespace clz::editor::backend
{
	bool init()
	{
		if (!initializeEditorPipeline())
		{
			clz::log::error("Failed to initialize editor's pipeline");
			return false;
		}


		clz::log::info("Initialized editor's backend");
		return true;
	}

	void flagBackendComponentsLoaded()
	{
		updateTextureBufferObject();
	}

	void shutdown()
	{
		destroyEditorPipeline();
	}
}