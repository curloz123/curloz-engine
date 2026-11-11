#include "../../../include/offscreen/backend/backend.hpp"
#include "core/logs.hpp"

namespace clz::editor::backend
{
	bool init()
	{
		if (!bodyEditorPipeline.initialize())
		{
			clz::log::error(
				"unable to initialize editor backend",
				true
			);
			return false;
		}

		return true;
	}

	void shutdown()
	{
		bodyEditorPipeline.destroy();
	}
} // namespace clz::editor::backend
