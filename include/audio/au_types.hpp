#pragma once

#include <AL/alc.h>
#include <AL/al.h>
#include <unordered_map>

namespace clz::audio
{
	// Global variables
	inline ALCdevice* au_device = nullptr;
	inline ALCcontext* au_context = nullptr;

	// Buffer storage
	inline std::unordered_map<const char*, ALuint> au_bufferIndices;
} // namespace clz::audio
