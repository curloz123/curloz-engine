/**
 * @file native.cpp
 * @author curl0z
 * @brief Audio system's backend(OpenAL) implementation
 */

#include "audio/native.hpp"
#include "al.h"
#include "alc.h"
#include "core/logs.hpp"
#include <expected>
#include <vector>

/// define stb vorbis definition
#define STB_VORBIS_IMPLEMENTATION
#include <stb_vorbis.c>

/// define dr_wav implementation
#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

namespace clz::audio
{
	/// @copydoc initializeOpenAL
	bool initializeOpenAL()
	{
		au_device = alcOpenDevice(nullptr);
		if (!au_device)
		{
			clz::log::error("OpenAL unable to create audio device handle");
			return false;
		}

		au_context = alcCreateContext(au_device, nullptr);
		if (!au_context)
		{
			clz::log::error("OpenAL unable to create audio context");
			return false;
		}
		if (!alcMakeContextCurrent(au_context))
		{
			clz::log::error("OpenAL unable make out context current lmao");
			return false;
		}

		return true;
	}

	/// @copydoc closeOpenAL
	void closeOpenAL()
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(au_context);
		alcCloseDevice(au_device);
	}

	/// @copydoc loadWav
	std::expected<ALuint, std::string> loadWav(const std::filesystem::path& audioFile)
	{
		ALuint buffer;
		alGenBuffers(1, &buffer);

		drwav wav;
		if (!drwav_init_file(&wav, audioFile.c_str(), nullptr))
		{
			clz::log::error("Dr Wav Could not parse audio file: " + audioFile.string());
			return std::unexpected("Could not parse audio file");
		}


		ALenum format;
		switch(wav.channels)
		{
			case(1):
				clz::log::debug("mono");
				format = AL_FORMAT_MONO16;
				break;
			case(2):
				clz::log::debug("stereo");
				format = AL_FORMAT_STEREO16;
				break;
			default:
				drwav_uninit(&wav);
				clz::log::error("Audio file: " + audioFile.string() + 
						" has unknown number of channels." + 
						" Supported are only MONO16 and STEREO16");
				return std::unexpected("Could not parse audio file");
		}

		ALsizei sampleRate = wav.sampleRate;

		const size_t sampleSize = wav.totalPCMFrameCount * wav.channels;
		std::vector<std::int16_t> samples(sampleSize);
		drwav_read_pcm_frames_s16(
			&wav,
			wav.totalPCMFrameCount,
			samples.data()
		);

		alBufferData(
			buffer, 
			format, 
			samples.data(), 
			samples.size() * sizeof(std::int16_t),
			sampleRate
		);

		drwav_uninit(&wav);

		return buffer;
	}
	
	/// @copydoc loadOgg
	std::expected<ALuint, std::string> loadOgg(const std::filesystem::path& audioFile)
	{
		ALuint buffer;
		alGenBuffers(1, &buffer);

		int channels, sampleRate;
		short* samples;
		int frameCount = stb_vorbis_decode_filename(
					audioFile.c_str(),
					&channels,
					&sampleRate,
					&samples
				);
		if (!samples)
		{
			clz::log::error("stb could not parse audio file: " + audioFile.string());
			return std::unexpected("Could not parse audio file");
		}

		ALenum format;
		switch(channels)
		{
			case(1):
				format = AL_FORMAT_MONO16;
				break;
			case(2):
				format = AL_FORMAT_STEREO16;
				break;
			default:
				clz::log::error("Audio file: " + audioFile.string() + 
						" has unknown number of channels." + 
						" Supported are only MONO16 and STEREO16");
				return std::unexpected("Could not parse audio file");
		}

		alBufferData(
			buffer,
			format,
			samples,
			frameCount * channels * sizeof(short),
			sampleRate
		);

		free(samples);

		return buffer;
	}

}
