/**
 * @file config.hpp
 * @author curl0z
 * @brief Stores all config data for for window subsystem.
 * Hint -> Basically all the part under [window] section in engine.toml
 */

#pragma once

#include "config/config.hpp"

namespace clz::window
{
	///< @brief Parsed width value from config
	///< Stored under [window][width]
	inline int w_width = 800;

	///< @brief Parsed height value from config.
	///< Stored under [window][height]
	inline int w_height = 600;

	///< @brief Determines whether to enable raw input.
	///< Stored under [window][enable_raw_input].
	inline bool w_enableRawInput = true;

	///< @brief Determines whether to make window exclusively fullscreen
	///< Stored under [window][exclusive_fullscreen]
	inline bool w_exclusiveFullscreen = false;

	/// @brief Parses window subsystem data from config
	/// @note Must be called before performing doing anything in window::init
	inline void parseConfigData()
	{
		w_width  = clz::config::getValue<int>("window", "width", 800);
		w_height = clz::config::getValue<int>("window", "height", 600);

		w_enableRawInput = clz::config::getValue<bool>("window", "enable_raw_input", true);
		w_exclusiveFullscreen =
			clz::config::getValue<bool>("window", "exclusive_fullscreen", "false");
	}
}
