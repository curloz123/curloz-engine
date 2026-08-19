#include "../../include/system_settings/system_settings.hpp"
#include <imgui.h>
#include "renderer/postprocess/tonemap.hpp"
#include "../../include/editor_types.hpp"
#include "renderer/postprocess/tonemap.hpp"
#include "../../include/timemachine.hpp"

namespace
{
	static void showPostProcessSettings();
}
namespace clz::editor
{
	void showRenderSystemSettings()
	{
		ImGui::PushFont(fontMonoBold);
		if (!ImGui::BeginTabItem("Renderer"))
		{
			ImGui::PopFont();
			ImGui::EndTabItem();
			return;
		}
		ImGui::PopFont();

		showPostProcessSettings();

		ImGui::EndTabItem();
	}
}
namespace
{
	static void showPostProcessSettings()
	{
		static float exposure = clz::renderer::post_process::getExposure();
		static float prevExposure = clz::renderer::post_process::getExposure();
		static bool exposureChanged = false;	

		if (exposure != clz::renderer::post_process::getExposure())
			exposure = clz::renderer::post_process::getExposure();

		if (ImGui::SliderFloat("Exposure", &exposure, -10.0f, 10.0f))
		{
			exposureChanged = true;
		}
		if (ImGui::IsItemActivated())
		{
			prevExposure = clz::renderer::post_process::getExposure();
		}
		if (ImGui::IsItemDeactivated())
		{
			exposureChanged = true;
			const float oldExposure = prevExposure;
			const float newExposure = exposure;

			clz::timemachine::createSnapshot(
				[oldExposure]()
				{
					clz::renderer::post_process::setExposure(oldExposure);
				},
				[newExposure]()
				{
					clz::renderer::post_process::setExposure(newExposure);
				}
			);
		}
		if (exposureChanged)
		{
			clz::renderer::post_process::setExposure(exposure);
		}
	}
}
