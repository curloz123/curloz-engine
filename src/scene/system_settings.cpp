#include "scene/system_settings.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
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
			clz::renderer::post_process::setExposure(1.0f);
			clz::renderer::post_process::setVignette(0.3f, 0.7f);
		};

		if (renderSystemEntry.contains("post_process"))
		{
			const auto& post_processEntry = renderSystemEntry["post_process"];

			// load exposure
			if (post_processEntry.contains("exposure"))
			{
				float exposure = post_processEntry["exposure"];
				clz::renderer::post_process::setExposure(exposure);
			}
			else
			{
				clz::log::warn("scene file's render system's entry "
						"does not have post_process->exposure entry");
				clz::renderer::post_process::setExposure(1.0f);
			}

			// load vignette
			if (post_processEntry.contains("vignette"))
			{
				const float start = post_processEntry["vignette"][0];
				const float end = post_processEntry["vignette"][1];
				clz::renderer::post_process::setVignette(start, end);
			}
			else
			{
				clz::log::warn("scene file's render system's entry "
						"does not have post_process->vignette entry");
				clz::renderer::post_process::setVignette(0.3f, 0.7f);
			}

			// load chromatic aberration
			if (post_processEntry.contains("chromaticaberrationstrength"))
			{
				clz::renderer::post_process::setChromaticAberrationStrength(
					post_processEntry["chromaticaberrationstrength"]
				);
			}
			else
			{
				clz::log::warn("scene file's render system's entry "
						"does not have post_process->chromaticaberrationstrength entry");
				clz::renderer::post_process::setChromaticAberrationStrength(0.002f);
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
		
		const auto vignette = clz::renderer::post_process::getVignette();
		renderSystemEntry["post_process"]["vignette"][0] = vignette.x;
		renderSystemEntry["post_process"]["vignette"][1] = vignette.y;

		renderSystemEntry["post_process"]["chromaticaberrationstrength"] = clz::renderer::post_process::getChromaticAberrationStrength();
	}	
}
