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

	struct BloomPC
	{
		/// @brief u1->first time, u2->horizontal, u3->vertical
		uint32_t bloomBits;
		int bloomStrength;
	};
	struct Pre_TonemapPC
	{
		uint32_t postProcessBits;
	};
	struct TonemapPC
	{
		float exposure;
	};
	struct Post_TonemapPC
	{
		uint32_t postProcessBits;
		
		float aspectRatio;

		float vignetteStart;
		float vignetteEnd;

		float chromaticAberrationStrength;
	};
} // namespace clz::renderer
