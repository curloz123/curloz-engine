/**
 * @file audio.cpp
 * @author curl0z
 * @brief Loads all audio system data.
 * Like audio buffers, and buffer players
 */

#include "scene/data/audio.hpp"
#include "audio/buffer_manager.hpp"

namespace clz::scene
{
	/// @copydoc loadAudioBuffers
	void loadAudioBuffers(const nlohmann::json& auBufferArray)
	{
		for (const auto& bufferPath : auBufferArray)
		{
			audio::loadBuffer(
				std::filesystem::path(bufferPath)
			);
		}
	}

	/// @copydoc saveBackAudioBuffers
	void saveBackAudioBuffers(nlohmann::json& auBufferArray)
	{
		std::vector<std::string> audioBufferPaths;
		audio::getAllBuffersPath(audioBufferPaths);

		for (const auto& bufferPath : audioBufferPaths)
		{
			auBufferArray.push_back(bufferPath);
		}
	}



}
