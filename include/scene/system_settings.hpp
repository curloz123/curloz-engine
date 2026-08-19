#pragma once 

#include "nlohmann/json.hpp"

namespace clz::scene
{
	void loadSystemSettings(const nlohmann::json& systemSettingEntry);
	void saveSystemSettings(nlohmann::json& systemSettingEntry);
}
