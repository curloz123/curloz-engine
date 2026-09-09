#pragma once

#include <filesystem>

namespace clz::script
{
	bool init();
	bool loadScript(const std::filesystem::path& scriptPath);
	void shutdown();
}
