#pragma once

#include <AL/alc.h>
#include <expected>
#include <string>

namespace clz::audio
{
	std::expected<void, std::string>
	initializeOpenAL(ALCdevice*& rp_device, ALCcontext*& rp_context);
	void closeOpenAL(ALCdevice*& rp_device, ALCcontext*& rp_context);
} // namespace clz::audio