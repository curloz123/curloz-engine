#pragma once 

#include "math/vec3.hpp"
#include <AL/al.h>

namespace clz::audio 
{
	ALuint createAudioSource(const clz::math::vec3& sourcePosition);
	void deleteAudioSource(ALuint sourceID);

	void playSound(const ALuint sourceID, const ALuint buffer);
	void stop(const ALuint sourceID);
	bool isPlaying(const ALuint sourceID);
	void pause(const ALuint sourceID);
	void continuePlaying(const ALuint sourceID);
	void setLooping(const bool loop);
}
