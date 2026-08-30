/**
 * @file config.hpp
 * @author curl0z
 * @brief Parses, provides and saves back render system settings from config
 */

#pragma once

#include <vulkan/vulkan.h>
#include "config/config.hpp"
#include <array>
#include "renderer/postprocess/bloom.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/postprocess/tonemap.hpp"
#include "renderer/utility/devicefeatures.hpp"

namespace clz::renderer
{
	/// @brief enable triple buffering?
	inline bool tripleBuffering;
	/// @brief MSAA value
	inline VkSampleCountFlagBits MSAA;
	
	/// @brief Exposure value
	inline float exposure;
	/// @brief chromatic aberration 
	inline float chromaticAberration;

	/// @brief Vignette value
	/// index 0 is near, 1 is far
	inline std::array<float, 2> vignette;

	/// @brief bloom strength
	inline float bloomStrength;

	/// @brief Bloom's filter radius
	inline float bloomFilterRadius;

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
		tripleBuffering = clz::config::getValue<bool>("renderer", "triple_buffering", false);
		MSAA = getMsaaFlagBitsFromInt(
				clz::config::getValue<uint32_t>("renderer", "msaa", 4));
		clz::log::debug("MSAA value: " + std::to_string(clz::config::getValue<uint32_t>("renderer", "msaa", 4)));
	
		exposure = clz::config::getValue<float>("renderer", "post_process", "exposure", 1.0f);
		clz::log::debug("exposure value: " + std::to_string(exposure));
		chromaticAberration = clz::config::getValue<float>("renderer", "post_process", "chromatic_aberration", 0.0f);
		clz::log::debug("CA value: " + std::to_string(chromaticAberration));
		vignette = 
		{
			clz::config::getValue<float>("renderer", "post_process", "vignette_near", 0.3f),
			clz::config::getValue<float>("renderer", "post_process", "vignette_far", 0.9f)
		};
		bloomStrength = clz::config::getValue<float>("renderer", "post_process", "bloom_strength", 0.15f);
		clz::log::debug("BS value: " + std::to_string(bloomStrength));
		bloomFilterRadius = clz::config::getValue<float>("renderer", "post_process", "bloom_filter_radius", 0.01f);
		clz::log::debug("BFR value: " + std::to_string(bloomFilterRadius));
	}

	/**
	 * @brief Returns whether triple buffering is enabled
	 * @return true if triple buffering is enabled, false otherwise
	 */
	inline bool getTripleBufferingFromConfig() { return tripleBuffering; }

	/**
	 * @brief Returns the configured MSAA sample count
	 * @return the MSAA setting as a VkSampleCountFlagBits
	 */
	inline VkSampleCountFlagBits getMSAAFromConfig() { return MSAA; }

	/**
	 * @brief Returns the configured post-process exposure value
	 * @return exposure value
	 */
	inline float exposureFromConfig() { return exposure; }

	/**
	 * @brief Returns the configured chromatic aberration strength
	 * @return chromatic aberration strength
	 */
        inline float chromaticAberrationFromConfig() { return chromaticAberration; }

	/**
	 * @brief Returns the configured vignette near value
	 * @return vignette near value
	 */
        inline float vignetteNearFromConfig() { return vignette[0]; }

	/**
	 * @brief Returns the configured vignette end value
	 * @return vignette end value
	 */
        inline float vignetteEndFromConfig() { return vignette[1]; }

	/**
	 * @brief Returns the configured bloom strength
	 * @return bloom strength
	 */
        inline float bloomStrengthFromConfig() { return bloomStrength; }

	/**
	 * @brief Returns the configured bloom filter radius
	 * @return bloom filter radius
	 */
        inline float bloomFilterRadiusFromConfig() { return bloomFilterRadius; }

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
                clz::config::writeValue("renderer", "triple_buffering", tripleBuffering);
                clz::config::writeValue<uint32_t>("renderer", "msaa", getIntFromMsaaFlagBits(MSAA));

                // [renderer.post_process]
                clz::config::writeValue("renderer", "post_process", "exposure", 
				clz::renderer::post_process::getExposure());
                clz::config::writeValue("renderer", "post_process", "chromatic_aberration", 
				clz::renderer::post_process::getChromaticAberrationStrength());
		const auto vignette = clz::renderer::post_process::getVignette(); 
		clz::config::writeValue("renderer", "post_process", "vignette_near", vignette.x);
                clz::config::writeValue("renderer", "post_process", "vignette_end", vignette.y);
                clz::config::writeValue("renderer", "post_process", "bloom_strength", 
				clz::renderer::post_process::getBloomStrength());
                clz::config::writeValue("renderer", "post_process", "bloom_filter_radius", 
				clz::renderer::post_process::getFilterRadius());
        }
}
