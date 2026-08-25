#include "audio/audio.hpp"
#include "audio/au_types.hpp"
#include "audio/native.hpp"
#include "core/logs.hpp"
#include <AL/alc.h>

namespace clz::audio
{
	void init()
	{
		if (auto result = initializeOpenAL(au_device, au_context); !result)
		{
			clz::log::error(result.error());
		}
		clz::log::debug("Initialized audio");
	}

	void shutdown()
	{
		closeOpenAL(au_device, au_context);
		clz::log::debug("Shut down audio");
	}
} // namespace clz::audio
