/**
 * @file lights.hpp
 * @author curl0z
 * @brief Raw GPU-facing light data structures.
 *
 * These structs mirror the memory layout expected by the shaders
 * (see mainpipeline.frag) and are uploaded directly via UBO/SSBO.
 * Do not add virtual functions or non-trivial members here — layout
 * must stay POD-compatible with std140/std430 rules.
 */

#pragma once

#include "math/vec3.hpp"
#include "math/vec4.hpp"

namespace clz::renderer
{

	/**
	 * @brief Identifies which kind of light a given entry represents.
	 * @note Currently used for editor/tooling purposes; the actual GPU
	 * buffers are split into separate arrays per type (see LightArrays),
	 * not a single tagged-union array.
	 */
	enum class LightType
	{
		DIRECTIONAL,
		POINT,
		SPOT
	};

	/**
	 * @brief GPU-layout directional (sun-like) light.
	 *
	 * A directional light has no position — it's treated as coming from
	 * infinitely far away, uniformly across the whole scene, along
	 * @c direction.
	 */
	struct DirectionalLight
	{
		/// @brief direction of directional light
		/// x,y,z decides direction
		/// w is just for padding
		math::vec4 direction = math::vec4(0.0f, -1.0f, 0.0f, 0.0f);

		/// @brief color of directional light
		/// x,y,z decides direction
		/// w decides intensity
		math::vec4 color = math::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		DirectionalLight() = default;
	};

	/**
	 * @brief GPU-layout point (omnidirectional) light.
	 *
	 * Radiates equally in all directions from @c position and falls off
	 * with distance according to @c attenuation, up to @c range.
	 */
	struct PointLight
	{
		/// @brief positon of point light
		/// x,y,z decides position
		math::vec3 position = math::vec3(0.0f, 0.0f, 0.0f);
		/// @brief decides range of point light
		float range = 10.0f;

		/// @brief color of point light
		/// x,y,z decides color
		math::vec3 color = math::vec3(1.0f, 1.0f, 1.0f);
		/// @brief intensity of light
		float intensity = 1.0f;

		/**
		 * @brief Attenuation values
		 * Below is the table for attenuation values

			Distance	Constant 	Linear 	Quadratic
			7 		1.0 		0.7 	1.8
			13 		1.0 		0.35 	0.44
			20 		1.0 		0.22 	0.20
			32 		1.0 		0.14 	0.07
			50 		1.0 		0.09 	0.032
			65 		1.0 		0.07 	0.017
			100 		1.0 		0.045 	0.0075
			160 		1.0 		0.027 	0.0028
			200 		1.0 		0.022 	0.0019
			325 		1.0 		0.014 	0.0007
			600 		1.0 		0.007 	0.0002
			3250 		1.0 		0.0014 	0.000007

		 * x component decides constant (always 1.0f)
		 * y component decides linear value
		 * z component decides quadratic value
		 * w is just padding
		 *
		 * Formula for attenuation is:
		 * 1.0f / (Kc + Kl*d + Kq*(d^2))
		 * @note Constant is always kept 1.0
		 * @warning If the light sits extremely close to (or on) a surface,
		 * @c d approaches 0 and this formula approaches 1/Kc, which is
		 * finite and safe — but combined with a near-zero-roughness
		 * specular term in the shader, the resulting radiance can still
		 * overflow to Inf/NaN. Clamp distance on the shader side, not here.
		 */
		math::vec4 attenuation = math::vec4(1.0f, 0.09f, 0.032f, 1.0f);

		PointLight() = default;
	};

	/**
	 * @brief GPU-layout spot light.
	 *
	 * A point light additionally constrained to a cone defined by
	 * @c direction and @c cutoff (inner/outer angles for soft edges).
	 * @note Not yet consumed by the fragment shader — SpotLight data is
	 * uploaded to LightsDataUBO's numSpotLights count but there's no
	 * SSBO binding or shading loop for it yet (see mainpipeline.frag).
	 */
	struct SpotLight
	{
		/// @brief direction of spotlight
		/// x,y,z decides direction
		/// w is just for padding
		math::vec4 direction = math::vec4(0.0f, 0.0f, -1.0f, 0.0f);

		/// @brief position of spotlight
		/// x,y,z decides position
		/// w decides range
		math::vec4 position = math::vec4(0.0f, 0.0f, 0.0f, 10.0f);

		/// @brief color of point light
		/// x,y,z decides color
		/// w decides intensity
		math::vec4 color = math::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		/**
		 * @brief Attenuation values
		 * Below is the table for attenuation values

			Distance	Constant 	Linear 	Quadratic
			7 		1.0 		0.7 	1.8
			13 		1.0 		0.35 	0.44
			20 		1.0 		0.22 	0.20
			32 		1.0 		0.14 	0.07
			50 		1.0 		0.09 	0.032
			65 		1.0 		0.07 	0.017
			100 		1.0 		0.045 	0.0075
			160 		1.0 		0.027 	0.0028
			200 		1.0 		0.022 	0.0019
			325 		1.0 		0.014 	0.0007
			600 		1.0 		0.007 	0.0002
			3250 		1.0 		0.0014 	0.000007

		 * x component decides constant (always 1.0f)
		 * y component decides linear value
		 * z component decides quadratic value
		 * w is just padding
		 *
		 * Formula for attenuation is:
		 * 1.0f / (Kc + Kl*d + Kq*(d^2))
		 * @note Constant is always kept 1.0
		 */
		math::vec4 attenuation = math::vec4(1.0f, 0.09f, 0.032f, 1.0f);

		/// @brief spotlight cutoff values
		/// x decides inner cutoff angle
		/// y decides outer cutoff angle
		math::vec4 cutoff = math::vec4(25.0f, 35.0f, 0.0f, 1.0f);

		SpotLight() = default;
	};

} // namespace clz::renderer