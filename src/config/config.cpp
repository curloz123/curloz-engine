/**
 * @file config.cpp
 * @author curl0z
 * @brief Engine configuration loader implementation.
 */

#define TOML_EXCEPTIONS 0
#include "config/config.hpp"
#include "core/logs.hpp"
#include <fstream>
#include <toml++/toml.hpp>

namespace clz::config
{
	constexpr auto engineConfigFile = "config/engine.toml";
	/// @copydoc init
	bool init()
	{
		auto result = toml::parse_file(engineConfigFile);
		if (!result) [[unlikely]]
		{
			clz::log::error(
				"Failed to parse engine configuration file: "
				"config/engine.toml"
			);
			return false;
		}
		cfg_config = std::move(result.table());
		clz::log::info("Config loaded");

		return true;
	}

	/// @copydoc shutdown
	void shutdown()
	{
		std::ofstream file(engineConfigFile);
		file << cfg_config;
		clz::log::info("Saved engine config file");
	}

} // namespace clz::config
