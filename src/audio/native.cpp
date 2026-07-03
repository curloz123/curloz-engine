#include "audio/native.hpp"
#include "core/logs.hpp"
#include <AL/al.h>

namespace clz::audio
{
	std::expected<void, std::string> initializeOpenAL(ALCdevice*& rp_device, ALCcontext*& rp_context)
	{
		rp_device = alcOpenDevice(nullptr);
		if (!rp_device)
		{
			clz::log::error("Could not open audio device");
			return std::unexpected("Could not open audio device");
		}

		rp_context = alcCreateContext(rp_device, nullptr);
		if (!rp_context)
		{
			clz::log::error("Could not create audio context");
			return std::unexpected("Could not create audio context");
		}

		if (!alcMakeContextCurrent(rp_context))
		{
			clz::log::error("Could not make context current");
			return std::unexpected("Could not make context current");
		}
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
		alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		constexpr float orientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
		alListenerfv(AL_ORIENTATION, orientation);
		clz::log::warn("temporarily setting listener to origin, looking at -ve z axis, "
			       "change this afterwards");

		return {};
	}
	void closeOpenAL(ALCdevice*& rp_device, ALCcontext*& rp_context)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(rp_context);
		alcCloseDevice(rp_device);
	}
} // namespace clz::audio