/**
 * @file logs.hpp
 * @author curl0z
 * @brief Simple logging utility for Curloz Engine.
 *
 * Provides info, warn and error logging via std::println.
 * Errors set hadError to true which can be checked by the caller.
 */

#pragma once

#include <print>
#include <source_location>
#include <string_view>

namespace clz::log
{
	/// @brief ANSI color codes for terminal output.
	namespace color
	{
		constexpr auto reset = "\033[0m";
		constexpr auto red = "\033[31m";
		constexpr auto yellow = "\033[33m";
		constexpr auto blue = "\033[34m";
		constexpr auto green = "\033[32m";
	} // namespace color

	/// @brief Set to true when error() is called. Never resets automatically.
	inline bool hadError = false;

	/**
	 * @brief Are basic logs or informational messages
	 * @param info string_view type information
	 */
	inline void info(std::string_view info)
	{
		std::println(
			"{}[INFO]{} {}",
			color::green,
			color::reset,
			info);
	}

	/**
	 * @brief Logs a warning message.
	 * @param warn Warning.
	 * @param loc Retrieves location of caller, don't pass this yourself
	 */
	inline void warn(
		std::string_view warn,
		const std::source_location loc =
			std::source_location::current())
	{
		std::println(
			"{}[WARN]{} {}",
			color::yellow,
			color::reset,
			warn);

		std::println(
			"LOCATION: {} Line:{}",
			loc.file_name(),
			loc.line());
	}

	/**
	 * @brief Logs a debug message.
	 * @param debug message
	 */
	inline void debug(
		std::string_view debug,
		const std::source_location loc =
			std::source_location::current())
	{
		std::println(
			"{}[DEBUG]{} {}",
			color::blue, color::reset, debug);
		std::println(
			"LOCATION: {} Line:{}",
			loc.file_name(),
			loc.line());
	}

	/**
	 * @brief Logs an error message and sets hadError to true.
	 * @param error The error
	 * @param loc Retrieves location of caller, don't pass this yourself
	 */
	inline void error(
		std::string_view error,
		const std::source_location loc =
			std::source_location::current())
	{
		hadError = true;

		std::println(
			"{}[ERROR]{} {}",
			color::red,
			color::reset,
			error);

		std::println(
			"LOCATION: {} Line:{}",
			loc.file_name(),
			loc.line());
	}

	/**
	 * @brief Returns true if an error has been logged.
	 */
	inline bool errorOccurred()
	{
		return hadError;
	}

} // namespace clz::log