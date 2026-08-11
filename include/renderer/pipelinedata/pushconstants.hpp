/**
 * @file pushconstants.hpp
 * @author curl0z
 * @brief Defines all push constants
 * to be used in pipelines
 */
#pragma once

#include "math/mat4x4.hpp"
#include "math/vec4.hpp"

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

		/// @brief normal texture id
		uint32_t normalTextureIndex;
	};

} // namespace clz::renderer