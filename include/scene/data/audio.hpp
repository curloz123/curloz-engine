/**
 * @file audio.hpp
 * @author curl0z
 * @brief Loads all audio system data. 
 * Like audio buffers, and buffer players
 */

#pragma once

#include <nlohmann/json.hpp>

namespace clz::scene
{
	/// @brief Load all audio buffers
	/// Present in JSON's "audio_buffer" array
	/// @param auBufferArray Audio buffer json array
	void loadAudioBuffers(const nlohmann::json& auBufferArray);

	/// @brief Saves back all audio buffers
	/// back to "audio_buffer" array
	/// @param auBufferArray Audio buffer json array, passed by reference
	void saveBackAudioBuffers(nlohmann::json& auBufferArray);
}
