#include "../../../include/offscreen/backend/backend.hpp"
#include "../../../include/offscreen/backend/descriptor.hpp"
#include "../../../include/offscreen/backend/pipeline.hpp"
#include "../../../include/offscreen/backend/ubo.hpp"
#include "core/logs.hpp"

namespace clz::editor::backend
{
bool init()
{

	/// --- Create Editor's UBO first
	if (!createEditorUniformBuffers())
	{
		clz::log::error("Could not create editor's backend uniform buffers!");
		return false;
	}
	/// --- Create Editor pipeline's descriptors first
	if (!createEditorDescriptors())
	{
		clz::log::error("Could not create editor's backend descriptors!");
		return false;
	}

	if (!initializeEditorPipeline())
	{
		clz::log::error("Failed to initialize editor's pipeline");
		return false;
	}

	clz::log::info("Initialized editor's backend");
	return true;
}

void shutdown()
{
	destroyEditorPipeline();
	destroyEditorDescriptors();
	destroyEditorUniformBuffers();
}
} // namespace clz::editor::backend