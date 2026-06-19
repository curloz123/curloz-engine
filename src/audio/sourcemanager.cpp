#include "audio/sourcemanager.hpp"
#include "al.h"

namespace clz::audio 
{
	ALuint createAudioSource(const clz::math::vec3& sourcePosition)
	{
		ALuint sourceID;
		alGenSources(1, &sourceID);
		alSourcef(sourceID, AL_GAIN, 1.0f);
		alSourcef(sourceID, AL_PITCH, 1.0f);
		alSource3f(sourceID, AL_POSITION, 
				sourcePosition.x, sourcePosition.y, sourcePosition.z);

		return sourceID;
	}

	void playSound(const ALuint sourceID, const ALuint buffer)
	{
		alSourceStop(sourceID);
		alSourcei(sourceID, AL_BUFFER, buffer);
		alSourcePlay(sourceID);
	}

	void stop(const ALuint sourceID)
	{
		alSourceStop(sourceID);
	}

	bool isPlaying(const ALuint sourceID)
	{
		ALint state;
		alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}

	void pause(const ALuint sourceID)
	{
		alSourcePause(sourceID);
	}

	void continuePlaying(const ALuint sourceID)
	{
		alSourcePlay(sourceID);
	}

	void setLooping(const ALuint sourceID, const bool loop)
	{
		alSourcei(sourceID, AL_LOOPING, 
				loop ? AL_TRUE : AL_FALSE);
	}



}
