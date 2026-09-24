/**
 * @file buffer_manager.cpp
 * @author curl0z
 * @brief Implementation of buffer related functions
 */

#include "audio/buffer_manager.hpp"
#include "audio/native.hpp"
#include "core/logs.hpp"

namespace clz::audio
{
	/// @copydoc loadBuffer
	BufferId loadBuffer(const std::filesystem::path& audioFile)
	{
		if (auto it = au_pathToBufferIndexMap.find(audioFile.string()); 
			it != au_pathToBufferIndexMap.end())
		{
			return BufferId(it->second);
		}

		if (!std::filesystem::exists(audioFile))
		{
			clz::log::error("Audio file: " + audioFile.string() + 
					", does not exist");
			return BufferId();
		}

		auto registerBuffer = [&](const ALuint& buffer)
		{
			const uint32_t newBufferIndex = au_bufferLUT.size();
			au_bufferLUT.push_back(buffer);
			au_pathToBufferIndexMap[audioFile.string()] = newBufferIndex;
			return BufferId(newBufferIndex);
		};

		std::string fileExtension = audioFile.extension().string();
		if (fileExtension == ".ogg")
		{
			auto loadResult = loadOgg(audioFile);
			if (loadResult)
			{
				BufferId newBufferId = registerBuffer(loadResult.value());
				return newBufferId;
			}
			else
			{
				clz::log::error(loadResult.error());
				/// By default ID is null initialized
				return BufferId();
			}

		}
		else if (fileExtension == ".wav")
		{
			auto loadResult = loadWav(audioFile);
			if (loadResult)
			{
				BufferId newBufferId = registerBuffer(loadResult.value());
				return newBufferId;
			}
			else
			{
				clz::log::error(loadResult.error());
				/// By default ID is null initialized
				return BufferId();
			}
		}

		clz::log::error("Audio file: " + audioFile.string() + 
				" has unknown file format." + 
				" Supported are: '.ogg', '.wav'");

		return BufferId();
	}

	/// @copydoc deleteAllBuffers
	void deleteAllBuffers()
	{
		alDeleteBuffers(
			au_bufferLUT.size(), 
			au_bufferLUT.data());
		au_pathToBufferIndexMap.clear();
	}
}
