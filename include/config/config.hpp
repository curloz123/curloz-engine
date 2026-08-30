/**
 * @file config.hpp
 * @author curl0z
 * @brief Engine configuration system.
 *
 * Loads and parses engine.toml at startup and provides typed
 * getters for reading configuration values by section and key.
 *
 * @note The init of this subsystem must be called "First", ie. before any other's init is
 * called Usage:
 * @code
 * clz::config::init();
 * int width = clz::config::getInt("window", "width", 1920);
 * std::string name = clz::config::getString("engine", "name", "Unknown");
 * @endcode
 */

#pragma once

#include <string>
#include <toml++/toml.hpp>
#include "core/logs.hpp"

namespace clz::config
{
	/// @brief Parsed TOML configuration table.
	inline toml::table cfg_config;

	/**
	 * @brief Loads and parses config/engine.toml.
	 *
	 * Must be called before any getter. Logs an error and returns
	 * early if the file is missing or malformed.
	 */
	bool init();

	/**
	 * @brief You can make any changes mid-run
	 * All the changes will be written back to the config
	 * @note Changes have to be manually saved by calling the write function by you,
	 * we ain't doing anything ourselves
	 */
	void shutdown();

	template<typename T>
	T getValue(
		std::string_view section,
		std::string_view key,
		const T& defaultVal
	)
	{
		const auto node = cfg_config[section][key];
		if (!node)
		{
			clz::log::warn(
				"Config value not found: " + 
				std::string(section) +  "[" + std::string(key) + "]"
			);
			return defaultVal;
		}

		if (const auto value = node.value<T>(); value.has_value())
			return value.value();

		clz::log::warn(
			"some error happened while parsing: " + 
			std::string(section) + "[" + std::string(key) + "]" + 
			"\nMaybe you queried this value via wrong data type??"
		);
		return defaultVal;
	}

	template<typename T>
	void writeValue(
		std::string_view section,
		std::string_view key,
		const T& value
	)
	{
		toml::table* sec = cfg_config[section].as_table();
		if (!sec)
		{
			cfg_config.insert_or_assign(section, toml::table{});
			sec = cfg_config[section].as_table();
		}
		sec->insert_or_assign(key, value);
	}

	template<typename T>
	T getValue(
		std::string_view section,
		std::string_view subSection,
		std::string_view key,
		const T& defaultVal
	)
	{
		const auto node = cfg_config[section][subSection][key];
		if (!node)
		{
			clz::log::warn(
				"Config value not found: " + 
				std::string(section) +  "." + std::string(subSection) + 
				"[" + std::string(key) + "]"
			);
			return defaultVal;
		}

		if (const auto value = node.value<T>(); value.has_value())
			return value.value();

		clz::log::warn(
			"some error happened while parsing: " + 
			std::string(section) +  "." + std::string(subSection) + 
			"[" + std::string(key) + "]"
			"\nMaybe you queried this value via wrong data type??"
		);
		return defaultVal;
		
	}
	template<typename T>
	void writeValue(
		std::string_view section,
		std::string_view subSection,
		std::string_view key,
		const T& value
	)
	{
		toml::table* sec = cfg_config[section].as_table();
		if (!sec)
		{
			cfg_config.insert_or_assign(section, toml::table{});
			sec = cfg_config[section].as_table();
		}

		toml::table* sub = (*sec)[subSection].as_table();
		if (!sub)
		{
			sec->insert_or_assign(section, toml::table{});
			sub = (*sec)[subSection].as_table();

		}
		sub->insert_or_assign(key, value);
	}

} // namespace clz::config
