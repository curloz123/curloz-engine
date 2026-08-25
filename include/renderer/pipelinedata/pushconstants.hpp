/**
 * @file pushconstants.hpp
 * @author curl0z
 * @brief Defines all push constants
 * to be used in pipelines
 */
#pragma once

#include "math/mat4x4.hpp"
#include "math/vec4.hpp"
#include <cstdint>

namespace clz::renderer
{

	/// @brief Push constant whenever you need to draw a model
	struct ModelDataPC
	{
		/// @brief Model matrix of model
		math::mat4 modelMatrix;

		/// @brief Base color factor
		math::vec4 baseColorFactor;

		/// @brief base texture id value
		uint32_t baseTextureIndex;

		/// @brief metallic factor
		float metallicFactor;
		/// @brief roughness factor
		float roughnessFactor;
		/// @brief metallic-roughness texture id value
		uint32_t metallic_roughnessTextureIndex;

		/// @brief Emissive factor
		math::vec3 emissiveFactor;
		/// @brief emissive texture ID
		uint32_t emissiveTextureIndex;
		/// @brief emissive strength values
		float emissiveStrength;


		/// @brief normal texture id
		uint32_t normalTextureIndex;
	};

	/// @brief Bloom pipeline's push constant
	struct BloomPC
	{
		/**
		 * @brief Determines bloom processes
		 * 1u determines down-sampling.
		 * 2u determines up-sampling
		 * 3u determines disabled
		 */
		uint32_t bloomBits;

		/// @brief Determines which image to downsample right now
		/// @note If index is -1, render target will be used
		int32_t downIndex;

		/// @brief Determines which image to updample right now
		/// @note If index is -1, bloomed image should be used as attachment
		int32_t upIndex;

		/// @brief Filter-radius to use when up-sampling
		float filterRadius;

		/// @brief Bloom strength
		float bloomStrength;
	};

	/// @brief Push constant that'll be passed to tonemap pipelien and it's shaders
	struct TonemapPC
	{
		/// @brief Exposure value
		float exposure;
	};

	/// @brief Push constant that'll be passed to post-tonemap pipelien and it's shaders
	struct Post_TonemapPC
	{
		/// @brief bits determining which processes to enable
		uint32_t postProcessBits;
		
		/// @brief aspect ratio of render target
		float aspectRatio;

		/// @brief Vignette start value
		float vignetteStart;
		/// @brief Vignette end value
		float vignetteEnd;

		/// @brief Chromatic-aberration strength value
		float chromaticAberrationStrength;
	};
} // namespace clz::renderer
