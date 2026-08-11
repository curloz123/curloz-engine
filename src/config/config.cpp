/**
 * @file config.cpp
 * @author curl0z
 * @brief Engine configuration loader implementation.
 */

#define TOML_EXCEPTIONS 0
#include "config/config.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include <toml++/toml.hpp>

namespace clz::config
{
	/// @brief Parsed TOML configuration table.
	toml::table cfg_config;

	void init()
	{
		auto result = toml::parse_file("config/engine.toml");
		if (!result) [[unlikely]]
		{
			clz::log::error(
				"Failed to parse engine configuration file: "
				"config/engine.toml"
			);
			return;
		}
		cfg_config = std::move(result.table());
		clz::log::info("Config loaded");
	}

	int getInt(const std::string_view section, const std::string_view key, int defaultVal)
	{
		if (!cfg_config.contains(section))
		{
			clz::log::warn("Config section not found: " + std::string(section));
			return defaultVal;
		}

		auto sec = cfg_config[section];
		if (!sec.as_table()->contains(key))
		{
			clz::log::warn(
				"Config key not found: " + std::string(section) + "[" +
				std::string(key) + "]"
			);
			return defaultVal;
		}

		return cfg_config[section][key].value_or(defaultVal);
	}

	float getFloat(const std::string_view section, const std::string_view key, float defaultVal)
	{
		if (!cfg_config.contains(section))
		{
			clz::log::warn("Config section not found: " + std::string(section));
			return defaultVal;
		}

		auto sec = cfg_config[section];
		if (!sec.as_table()->contains(key))
		{
			clz::log::warn(
				"Config key not found: " + std::string(section) + "[" +
				std::string(key) + "]"
			);
			return defaultVal;
		}

		return cfg_config[section][key].value_or(defaultVal);
	}

	bool getBool(const std::string_view section, const std::string_view key, bool defaultVal)
	{
		if (!cfg_config.contains(section))
		{
			clz::log::warn("Config section not found: " + std::string(section));
			return defaultVal;
		}

		auto sec = cfg_config[section];
		if (!sec.as_table()->contains(key))
		{
			clz::log::warn(
				"Config key not found: " + std::string(section) + "[" +
				std::string(key) + "]"
			);
			return defaultVal;
		}

		return cfg_config[section][key].value_or(defaultVal);
	}

	std::string getString(
		const std::string_view section,
		const std::string_view key,
		const std::string_view defaultVal
	)
	{
		if (!cfg_config.contains(section))
		{
			clz::log::warn("Config section not found: " + std::string(section));
		}

		auto sec = cfg_config[section];
		if (!sec.as_table()->contains(key))
		{
			clz::log::warn(
				"Config key not found: " + std::string(section) + "[" +
				std::string(key) + "]"
			);
			return std::string(defaultVal);
		}

		return cfg_config[section][key].value_or(std::string(defaultVal));
	}

	std::string getAppName()
	{
		return clz::config::getString("engine", "name", "Unknown");
	}

	void printAppVersion()
	{
		const int major = clz::config::getInt("engine", "version_major", 0);
		const int minor = clz::config::getInt("engine", "version_minor", 0);
		const int patch = clz::config::getInt("engine", "version_patch", 0);

		clz::log::info(
			"Version: " + std::to_string(major) + "." + std::to_string(minor) + "." +
			std::to_string(patch)
		);
	}

} // namespace clz::config
