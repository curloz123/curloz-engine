#pragma once

#include <cassert>
#include <source_location>
#include <string_view>
#include <print>

namespace clz
{

	/// @brief ANSI color codes for terminal output.
	namespace color
	{
		constexpr const char* reset = "\033[0m";
		constexpr const char* red = "\033[31m";
		constexpr const char* yellow = "\033[33m";
		constexpr const char* blue = "\033[34m";
		constexpr const char* green = "\033[32m";
	} // namespace color

	inline void CLZ_ASSERT(const bool condition, std::string_view msg,
			       const std::source_location location = std::source_location::current())
	{
#ifdef CLZ_DEBUG
		if (condition)
			return;

		std::println("{}[ASSERT]{} {}", color::red, color::reset, msg);
		std::println("LOCATION: {} Line:{}", location.file_name(), location.line());
		assert(false);
#endif
	}
} // namespace clz
