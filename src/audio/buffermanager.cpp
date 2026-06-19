#include "audio/buffermanager.hpp"
#include "audio/au_types.hpp"
#include <cstdint>
#include <filesystem>
#include <limits>
#include <sys/types.h>
#include "al.h"
#include "core/logs.hpp"
#include <vector>
#include "dr_wav.h"

namespace clz::audio 
{
	ALuint loadWav(const std::string& filePath);
}

namespace clz::audio 
{
	ALuint loadBuffer(const std::string& filePath)
	{
		if (!std::filesystem::exists(filePath) || 
				!std::filesystem::is_regular_file(filePath))
		{
			clz::log::error("Unable to parse audio file: " + filePath);
			return std::numeric_limits<uint32_t>::max();
		}
		
		std::filesystem::path p = filePath;
		std::string fileExtension = p.extension().string();

		if (fileExtension == ".wav")
		{
			return loadWav(filePath);
		}

		clz::log::error("Unable to parse audio file: " + filePath);
		return std::numeric_limits<uint32_t>::max();
	}

	ALuint getBufferID(const char* fileName)
	{
		return au_bufferIndices[fileName];
	}

	void deleteBuffer(const ALuint buffer)
	{
		alDeleteBuffers(1, &buffer);
		clz::log::info("Deleted an audio buffer");
	}

	void deleteAllBuffers()
	{
		for (const auto& [name, buffer]: au_bufferIndices)
		{
			alDeleteBuffers(1, &buffer);
		}
		clz::log::info("Deleted all audio buffers");
	}
}

namespace clz::audio 
{
	ALuint loadWav(const std::string& filePath)
	{
		ALuint buffer;	///< Actual buffer that we're going to return
		alGenBuffers(1, &buffer);

		drwav wav;
		if (!drwav_init_file(&wav, filePath.c_str(), nullptr))
		{
			clz::log::error("Dr Wav Could not initialize file: " + filePath);
			return std::numeric_limits<uint32_t>::max();
		}

		// Samples
		std::vector<int16_t> samples(wav.totalPCMFrameCount * wav.channels); ///< Audio file's samples

		// Format
		ALenum format; ///< Format of audio file
		switch(wav.channels)
		{
			case(1):
				format = AL_FORMAT_MONO16;
				break;
			case(2):
				format = AL_FORMAT_STEREO16;
				break;
			default:
				clz::log::error("File: " + filePath + " has unknown number of channels");
				return std::numeric_limits<uint32_t>::max();
		}

		// Sample rate
		ALsizei sampleRate = wav.sampleRate; ///< Sample rate of audio file

		
		// Creating and returning the actual buffer
		alBufferData(buffer, format, samples.data(), samples.size() * sizeof(int16_t), sampleRate);

		// Register this audio buffer
		au_bufferIndices[filePath.c_str()] = buffer;


		return buffer;
	}
}
