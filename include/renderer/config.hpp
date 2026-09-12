/**
 * @file config.hpp
 * @author curl0z
 * @brief Parses, provides and saves back render system settings from config
 */

#pragma once

#include "config/config.hpp"
#include "renderer/postprocess/bloom.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/postprocess/tonemap.hpp"
#include "renderer/utility/devicefeatures.hpp"
#include "renderer/context/render_target_context.hpp"
#include "renderer/context/swapchaincontext.hpp"

namespace clz::renderer
{
	/**
	 * @brief Loads renderer settings from the config file into memory
	 *
	 * Reads the [renderer] and [renderer.post_process] tables via
	 * clz::config::getValue, falling back to sane defaults for any
	 * missing or malformed keys. Must be called before any of the
	 * getters below are used.
	 */
	inline void parseConfigData()
	{
		r_enableTripleBuffering = clz::config::getValue<bool>(
				"renderer", 
				"triple_buffering", 
				false);

		auto msaaInt = clz::config::getValue<uint32_t>(
				"renderer", 
				"msaa", 
				4);

		if (msaaInt > getIntFromMsaaFlagBits(getMaxMsaaSamples()))
		{
			clz::log::warn("engine.toml's renderer section "
					"has msaa value greater than "
					"supported by the gpu. Clamping it");
			MSAA = getMaxMsaaSamples();
		}
		else
		{
			MSAA = getMsaaFlagBitsFromInt(
					clz::config::getValue<uint32_t>(
						"renderer", 
						"msaa", 
						4));
		}
	
		post_process::setExposure(clz::config::getValue<float>(
				"renderer", 
				"post_process", 
				"exposure", 
				1.0f));
		post_process::setChromaticAberrationStrength(clz::config::getValue<float>(
				"renderer", 
				"post_process", 
				"chromatic_aberration", 
				0.0f));
		post_process::ChromaticAbberationEdgeFade edgeFade;
		edgeFade.near = clz::config::getValue<float>(
					"renderer",
					"post_process",
					"ca_edgefade_near",
					0.35f
				);
		edgeFade.far = clz::config::getValue<float>(
					"renderer",
					"post_process",
					"ca_edgefade_far",
					0.5f
				);
		post_process::setCAEdgeFade(edgeFade);

		post_process::setVignette(
			clz::config::getValue<float>(
					"renderer", 
					"post_process", 
					"vignette_near", 
					0.3f),
			clz::config::getValue<float>(
					"renderer", 
					"post_process", 
					"vignette_far", 
					0.9f));
		post_process::setBloomStrength(clz::config::getValue<float>(
				"renderer", 
				"post_process", 
				"bloom_strength", 
				0.15f));
		post_process::setBloomFilterRadius(clz::config::getValue<float>(
				"renderer", 
				"post_process", 
				"bloom_filter_radius", 
				0.01f));
	}

	/**
	 * @brief Writes the current renderer/post-process state back to the config file
	 *
	 * Pulls the live values from clz::renderer::post_process and writes
	 * them into the [renderer] and [renderer.post_process] tables via
	 * clz::config::writeValue. Only updates the in-memory config table;
	 * the caller is responsible for persisting it to disk afterward.
	 */
        inline void writeBackConfigData()
        {
                // [renderer]
                clz::config::writeValue(
				"renderer", 
				"triple_buffering", 
				r_enableTripleBuffering);
                clz::config::writeValue<uint32_t>(
				"renderer", 
				"msaa", 
				getIntFromMsaaFlagBits(MSAA));

                // [renderer.post_process]
                clz::config::writeValue(
				"renderer", 
				"post_process", 
				"exposure", 
				clz::renderer::post_process::getExposure());

                clz::config::writeValue(
				"renderer", 
				"post_process", 
				"chromatic_aberration", 
				clz::renderer::post_process::getChromaticAberrationStrength());
		const auto caEdgeFade = clz::renderer::post_process::getCAEdgeFade();
		clz::config::writeValue(
				"renderer",
				"post_process",
				"ca_edgefade_near",
				caEdgeFade.near
				);
		clz::config::writeValue(
				"renderer",
				"post_process",
				"ca_edgefade_far",
				caEdgeFade.far
				);

		const auto vignette = clz::renderer::post_process::getVignette(); 
		clz::config::writeValue(
				"renderer", 
				"post_process", 
				"vignette_near", 
				vignette.x);
                clz::config::writeValue(
				"renderer", 
				"post_process", 
				"vignette_far", 
				vignette.y);

                clz::config::writeValue(
				"renderer", 
				"post_process", 
				"bloom_strength", 
				clz::renderer::post_process::getBloomStrength());
                clz::config::writeValue(
				"renderer", 
				"post_process", 
				"bloom_filter_radius", 
				clz::renderer::post_process::getBloomFilterRadius());
        }
}
