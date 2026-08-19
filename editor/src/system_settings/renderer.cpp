#include "../../include/system_settings/system_settings.hpp"
#include <imgui.h>
#include "renderer/postprocess/post_tonemap.hpp"
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


		static clz::math::vec2 vignette = clz::renderer::post_process::getVignette();
		static clz::math::vec2 prevVignette = clz::renderer::post_process::getVignette();
		bool vignetteChanged = clz::renderer::post_process::isVignetteEnabled();

		if (vignette != clz::renderer::post_process::getVignette())
			vignette = clz::renderer::post_process::getVignette();

		auto createVignetteSnapshot = []()
		{
			auto oldVignette = prevVignette;
			auto newVignette = vignette;
			clz::timemachine::createSnapshot(
				[oldVignette]()
				{
					clz::renderer::post_process::setVignette(oldVignette);	
				},
				[newVignette]()
				{
					clz::renderer::post_process::setVignette(newVignette);
				}
			);
		};

		ImGui::Text("Vignette");
		if (ImGui::SliderFloat("Inner Cutoff", &vignette.x, -2.0, 2.0))
			vignetteChanged = true;
		if (ImGui::IsItemActivated())
			prevVignette = clz::renderer::post_process::getVignette();
		if (ImGui::IsItemDeactivated())
			createVignetteSnapshot();
		if (ImGui::SliderFloat("Outer Cutoff", &vignette.y, -2.0, 2.0))
			vignetteChanged = true;
		if (ImGui::IsItemActivated())
			prevVignette = clz::renderer::post_process::getVignette();
		if (ImGui::IsItemDeactivated())
			createVignetteSnapshot();

		if (vignetteChanged)
			clz::renderer::post_process::setVignette(vignette);



		static float chromaticAberration = clz::renderer::post_process::getChromaticAberrationStrength();
		static float prevChromaticAberration = clz::renderer::post_process::getChromaticAberrationStrength();
		static bool chromaticAberrationChanged = false;

		if (chromaticAberration != clz::renderer::post_process::getChromaticAberrationStrength())
		{
			chromaticAberration = clz::renderer::post_process::getChromaticAberrationStrength();
		}
		auto createChromaticAberrationSnapshot = []()
		{
			auto oldChromaticAberration = prevChromaticAberration;
			auto newChromaticAberration = chromaticAberration;

			clz::timemachine::createSnapshot(
				[oldChromaticAberration]()
				{
					clz::renderer::post_process::setChromaticAberrationStrength(
						oldChromaticAberration
					);
				},
				[newChromaticAberration]()
				{
					clz::renderer::post_process::setChromaticAberrationStrength(
						newChromaticAberration
					);
				}
			);
		};

		ImGui::Text("Chromatic Aberration");

		if (ImGui::SliderFloat("Strength",&chromaticAberration, -0.1f, 0.1f))
			chromaticAberrationChanged = true;
		if (ImGui::IsItemActivated())
			prevChromaticAberration = clz::renderer::post_process::getChromaticAberrationStrength();
		if (ImGui::IsItemDeactivated())
			createChromaticAberrationSnapshot();

		if (chromaticAberrationChanged)
		{
			clz::renderer::post_process::setChromaticAberrationStrength(
				chromaticAberration
			);
		}

	}
}
