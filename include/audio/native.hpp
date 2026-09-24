/**
 * @file native.hpp
 * @author curl0z
 * @brief Audio system's backend(OpenAL) implementation
 */

#pragma once 

#include <AL/al.h>
#include <AL/alc.h>
#include <filesystem>
#include <expected>

namespace clz::audio
{
	///< @brief OpenAL device handle
	inline ALCdevice*  au_device  = nullptr;
	///< @brief OpenAL context handle
	inline ALCcontext* au_context = nullptr;

	/// @brief Initializes OpenAL
	/// @return true on success, else false
	bool initializeOpenAL();

	/// @brief Closes OpenAL
	void closeOpenAL();

	/// @brief Parses a .wav file
	/// @param audioFile Path to audio file, relative to engine's dir
	/// @return ALuint openal handle to buffer if parsing was succesful,
	/// else error string
	std::expected<ALuint, std::string> loadWav(const std::filesystem::path& audioFile);

	/// @brief Parses a .ogg file
	/// @param audioFile Path to audio file, relative to engine's dir
	/// @return ALuint openal handle to buffer if parsing was succesful,
	/// else error string
	std::expected<ALuint, std::string> loadOgg(const std::filesystem::path& audioFile);
}
