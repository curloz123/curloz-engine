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
#include "core/logs.hpp"
#include "native.hpp"
#include <vector>

namespace clz::audio
{
	///< @brief Global audio buffer LUT.
	inline std::vector<ALuint> au_bufferLUT;
	///< @brief Stores all buffers path
	inline std::vector<std::string> au_bufferPathLUT;
	
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

	/// @brief Retrieves list of all buffer paths
	/// inside passed parameter
	/// @param rBufferPaths All paths will be saved here
	void getAllBuffersPath(std::vector<std::string>& rBufferPaths);

	/// @brief Retrieves buffer id by file name
	/// @param fileName Path to buffer file
	/// @return BufferId Id of the queried buffer. 
	/// Returns null if buffer doesnt exist
	inline BufferId getBufferIdByFileName(const std::string& fileName)
	{
		auto it = au_pathToBufferIndexMap.find(fileName);
		if (it != au_pathToBufferIndexMap.end())
		{
			return BufferId(it->second);
		}

		clz::log::warn("Tried to retrieve audio buffer: " + 
				fileName + 
				", but it doesn't exist:");
		return BufferId();
	}

	/// @brief Deletes all buffers
	void deleteAllBuffers();
}
