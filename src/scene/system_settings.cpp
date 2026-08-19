#include "scene/system_settings.hpp"
#include "renderer/postprocess/tonemap.hpp"
#include "core/logs.hpp"

namespace
{
	static void loadRendererSettings(const nlohmann::json& renderSystemEntry);
	static void saveRendererSettings(nlohmann::json& renderSystemEntry);
}
namespace clz::scene
{

	void loadSystemSettings(const nlohmann::json& systemSettingEntry)
	{
		for (const auto& systemSetting : systemSettingEntry)
		{
			if (systemSetting["system"] == "renderer")
			{
				loadRendererSettings(systemSetting);
			}
		}
	}

	void saveSystemSettings(nlohmann::json& sceneFile)
	{
		sceneFile["systemsettings"] = nlohmann::json::array();

		auto& renderSystemEntry = sceneFile["systemsettings"].emplace_back();
		saveRendererSettings(renderSystemEntry);
	}
}

namespace
{
	static void loadRendererSettings(const nlohmann::json& renderSystemEntry)
	{
		auto loadDefaultPost_Process = []()
		{
			clz::renderer::post_process::setExposure(1.0);
		};

		if (renderSystemEntry.contains("post_process"))
		{
			const auto& post_processEntry = renderSystemEntry["post_process"];
			if (post_processEntry.contains("exposure"))
			{
				float exposure = post_processEntry["exposure"];
				clz::renderer::post_process::setExposure(exposure);
			}
			else
			{
				clz::log::warn("scene file's renderer entry "
						"does not have post_process->exposure entry");
				clz::renderer::post_process::setExposure(1.0);
			}
		}
		else
		{
			clz::log::warn("scene file does not have renderer->post_process entry");
			loadDefaultPost_Process();
		}
	}
	static void saveRendererSettings(nlohmann::json& renderSystemEntry)
	{
		renderSystemEntry["system"] = "renderer";
		renderSystemEntry["post_process"]["exposure"] = clz::renderer::post_process::getExposure();
	}		
}
