/**
 * @file config.hpp
 * @author curl0z
 * @brief Stores all config data for for window subsystem.
 * Hint -> Basically all the part under [window] section in engine.toml
 * @note Config values just denote the initial values, and should not be used afterwards
 */

#pragma once

#include "config/config.hpp"

namespace clz::window
{
	///< @brief Parsed width value from config
	///< Stored under [window][width]
	inline int w_configWidth = 800;

	///< @brief Parsed height value from config.
	///< Stored under [window][height]
	inline int w_configHeight = 600;

	///< @brief Determines whether to enable raw input.
	///< Stored under [window][enable_raw_input].
	inline bool w_configEnableRawInput = true;

	///< @brief Determines whether to make window exclusively fullscreen
	///< Stored under [window][exclusive_fullscreen]
	inline bool w_configExclusiveFullscreen = false;

	/// @brief Parses window subsystem data from config
	/// @note Must be called before performing doing anything in window::init
	inline void parseConfigData()
	{
		w_configWidth  = clz::config::getValue<int>("window", "width", 800);
		w_configHeight = clz::config::getValue<int>("window", "height", 600);

		w_configEnableRawInput = clz::config::getValue<bool>("window", "enable_raw_input", true);
		w_configExclusiveFullscreen =
			clz::config::getValue<bool>("window", "exclusive_fullscreen", "false");
	}
}
