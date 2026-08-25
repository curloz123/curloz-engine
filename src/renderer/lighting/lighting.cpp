/**
 * @file lighting.cpp
 * @author curl0z
 * @brief Implementation of the light registry (see lighting.hpp).
 */

#include "renderer/lighting/lighting.hpp"

namespace clz::renderer
{
	/// @copydoc registerDirectionalLight
	std::expected<DirectionalLightId, LightRegisterError> registerDirectionalLight(
		const math::vec3& direction,
		const math::vec3& color,
		const float intensity
	)
	{
		if (numDirectionalLights >= MAX_DIR_LIGHTS)
		{
			return std::unexpected(LightRegisterError::LIMIT_EXCEEDED);
		}
		// Validate color channels are normalized and intensity sits in [0, 1].
		// @note this clamps intensity to at most 1.0 — unlike point lights,
		// there is currently no way to register a directional light with a
		// physically "brighter than one" intensity through this API.
		if (color.x < 0.0f || color.x > 1.0f || color.y < 0.0f || color.y > 1.0f ||
		    color.z < 0.0f || color.z > 1.0f || intensity < 0.0f)
		{
			return std::unexpected(LightRegisterError::INVALID_PARAMETER);
		}

		DirectionalLight directionalLight;
		directionalLight.direction =
			math::vec4(direction.x, direction.y, direction.z, 1.0f);

		directionalLight.color = math::vec4(color.x, color.y, color.z, intensity);

		Lights.directionalLight[numDirectionalLights] = std::move(directionalLight);
		DirectionalLightId lightId;
		lightId.value = numDirectionalLights;
		++numDirectionalLights;
		return lightId;
	}

	/// @copydoc registerPointLight(const math::vec3&,float,const math::vec3&,float,float,float)
	std::expected<PointLightId, LightRegisterError> registerPointLight(
		const math::vec3& position,
		const float range,
		const math::vec3& color,
		const float intensity,
		const float linearAttenuation,
		const float quadraticAttenuation
	)
	{
		if (numPointLights >= MAX_POINT_LIGHTS)
		{
			return std::unexpected(LightRegisterError::LIMIT_EXCEEDED);
		}
		// Unlike the directional light path, intensity here is only
		// bounds-checked from below — point lights are allowed arbitrarily
		// bright intensities since PBR radiance math expects large values.
		if (color.x < 0.0f || color.x > 1.0f || color.y < 0.0f || color.y > 1.0f ||
		    color.z < 0.0f || color.z > 1.0f || intensity < 0.0f || range < 0.0f)
		{
			return std::unexpected(LightRegisterError::INVALID_PARAMETER);
		}

		PointLight pointLight = {};
		pointLight.position = position;
		pointLight.range = range;
		pointLight.color = color;
		pointLight.intensity = intensity;
		pointLight.attenuation = math::vec4(
			1.0,		      // constant
			linearAttenuation,    // linear
			quadraticAttenuation, // quadratic
			-1.0
		);
		Lights.pointLights[numPointLights] = std::move(pointLight);
		PointLightId lightId;
		lightId.value = numPointLights;
		++numPointLights;
		return lightId;
	}

	/// @copydoc registerSpotLight
	std::expected<SpotLightId, LightRegisterError> registerSpotLight(
		const math::vec3& direction,
		const math::vec3& position,
		const float range,
		const math::vec3& color,
		const float intensity,
		const float linearAttenuation,
		const float quadraticAttenuation,
		const float innerCutoffAngle,
		const float outerCutoffAngle
	)
	{
		if (numSpotLights >= MAX_SPOT_LIGHTS)
		{
			return std::unexpected(LightRegisterError::LIMIT_EXCEEDED);
		}
		if (color.x < 0.0f || color.x > 1.0f || color.y < 0.0f || color.y > 1.0f ||
		    color.z < 0.0f || color.z > 1.0f || intensity < 0.0f || range < 0.0f)
		{
			return std::unexpected(LightRegisterError::INVALID_PARAMETER);
		}

		SpotLight spotLight = {};
		spotLight.direction = math::vec4(direction.x, direction.y, direction.z, 1.0f);
		spotLight.position = position;
		spotLight.range = range;
		spotLight.color = color;
		spotLight.intensity = intensity;
		spotLight.attenuation = math::vec2(linearAttenuation, quadraticAttenuation);
		spotLight.cutoff = math::vec2(innerCutoffAngle, outerCutoffAngle);

		Lights.spotLights[numSpotLights] = std::move(spotLight);
		SpotLightId lightId;
		lightId.value = numSpotLights;
		++numSpotLights;
		return lightId;
	}

} // namespace clz::renderer
