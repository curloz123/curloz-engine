/**
 * @file buffer_manager.hpp
 * @author curl0z
 * @brief Provides all buffer related functions
 */

#pragma once

#include "core/id_interface.hpp"
#include <string>
#include <unordered_map>
#include <filesystem>
#include "native.hpp"
#include <vector>

namespace clz::audio
{
	///< @brief Global audio buffer LUT.
	inline std::vector<ALuint> au_bufferLUT;
	
	///< @brief Path(std::string) -> buffer index(in LUT) Map.
	inline std::unordered_map<std::string, uint32_t> au_pathToBufferIndexMap;

	///< @brief Buffer ID type
	///< Indices the au_bufferLUT
	class BufferId : public IdInterface<>{};

	/// @brief Load audio file, and stores it as a buffer in LUT
	/// @param audioFile The path(relative to engine) of the audio file
	/// @return BufferId of newly created
	/// @note Always check whether buffer id is not or not
	BufferId loadBuffer(const std::filesystem::path& audioFile);

	/// @brief Deletes all buffers
	void deleteAllBuffers();
}
