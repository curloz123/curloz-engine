/**
 * @file system_settings.hpp
 * @author curl0z
 * @brief Loads subsystem's internal settings
 */
#pragma once 

#include "nlohmann/json.hpp"

namespace clz::scene
{
	void loadSystemSettings(const nlohmann::json& systemSettingEntry);
	void saveSystemSettings(nlohmann::json& systemSettingEntry);
}
