#include "../../include/system_settings/system_settings.hpp"
#include <imgui.h>
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/postprocess/tonemap.hpp"
#include "../../include/editor_types.hpp"
#include "renderer/postprocess/bloom.hpp"
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
		if (ImGui::BeginTabItem("Renderer"))
		{
			ImGui::PopFont();


			showPostProcessSettings();


			ImGui::EndTabItem();
		}
		else
		{
			ImGui::PopFont();
		}
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

		ImGui::Separator();
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

		ImGui::Separator();
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

		ImGui::Separator();
		ImGui::Text("Chromatic Aberration");

		if (ImGui::SliderFloat("Strength", &chromaticAberration, -0.5f, 0.5f))
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

		ImGui::Separator();

		static auto prevBloomStrength = clz::renderer::post_process::getBloomStrength();
		static auto prevFilterRadius = clz::renderer::post_process::getFilterRadius();
		ImGui::Text("Bloom");
		ImGui::Checkbox("Enable", &clz::renderer::post_process::Bloom);
		ImGui::SliderFloat("Filter Radius", &clz::renderer::post_process::filterRadius, 0.0005f, 0.05f);
		if (ImGui::IsItemActivated())
		{
			prevFilterRadius = clz::renderer::post_process::getFilterRadius();
		}
		if (ImGui::IsItemDeactivated())
		{
			auto oldFilterRadius = prevFilterRadius;
			auto newFilterRadius = clz::renderer::post_process::getFilterRadius();
			clz::timemachine::createSnapshot(
				[oldFilterRadius]()
				{
					clz::renderer::post_process::setFilterRadius(
						oldFilterRadius
					);
				},
				[newFilterRadius]()
				{
					clz::renderer::post_process::setFilterRadius(
						newFilterRadius
					);
				}
			);	
	
		}
		ImGui::SliderFloat("Bloom Strength", &clz::renderer::post_process::bloomStrength, 0.01, 0.6);
		if (ImGui::IsItemActivated())
		{
			prevBloomStrength = clz::renderer::post_process::getBloomStrength();
		}
		if (ImGui::IsItemDeactivated())
		{
			auto oldStrength = prevBloomStrength;
			auto newStrength = clz::renderer::post_process::getBloomStrength();

			clz::timemachine::createSnapshot(
				[oldStrength]()
				{
					clz::renderer::post_process::setBloomStrength(
						oldStrength
					);
				},
				[newStrength]()
				{
					clz::renderer::post_process::setBloomStrength(
						newStrength
					);
				}
			);	
	
		}
	}
}
