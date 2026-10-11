/**
 * @file audio.hpp
 * @author curl0z
 * @brief Audio system public header
 */

#pragma once

#include "native.hpp"

namespace clz::audio
{
	/// @brief Initializes Audio system
	/// @return true on success, false otherwise
	bool init();

	/// @brief Updates audio subsystem
	/// @note Won't update if editor mode is enabled
	void update();

	/// @brief Shuts down audio system
	void shutdown();
}
