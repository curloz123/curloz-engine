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
		if (Lights.directionalLight.size() >= MAX_DIR_LIGHTS)
		{
			return std::unexpected(LightRegisterError::LIMIT_EXCEEDED);
		}
		// Validate color channels are normalized and intensity sits in [0, 1].
		// @note this clamps intensity to at most 1.0 — unlike point lights,
		// there is currently no way to register a directional light with a
		// physically "brighter than one" intensity through this API.
		if (color.x < 0.0f || color.x > 1.0f || color.y < 0.0f || color.y > 1.0f ||
		    color.z < 0.0f || color.z > 1.0f || intensity < 0.0f || intensity > 1.0f)
		{
			return std::unexpected(LightRegisterError::INVALID_PARAMETER);
		}

		DirectionalLight directionalLight;
		directionalLight.direction =
			math::vec4(direction.x, direction.y, direction.z, 1.0f);

		directionalLight.color = math::vec4(color.x, color.y, color.z, intensity);

		Lights.directionalLight.push_back(directionalLight);
		DirectionalLightId lightId;
		lightId.value = Lights.directionalLight.size() - 1;
		return lightId;
	}

	/// @copydoc registerPointLight(const math::vec3&,float,const math::vec3&,float,float,float)
	std::expected<PointLightId, LightRegisterError> registerPointLight(
		const math::vec3& position,
		float range,
		const math::vec3& color,
		float intensity,
		float linearAttenuation,
		float quadraticAttenuation
	)
	{
		if (Lights.pointLights.size() >= MAX_POINT_LIGHTS)
		{
			return std::unexpected(LightRegisterError::LIMIT_EXCEEDED);
		}
		// Unlike the directional light path, intensity here is only
		// bounds-checked from below — point lights are allowed arbitrarily
		// bright intensities since PBR radiance math expects large values.
		if (color.x < 0.0f || color.x > 1.0f || color.y < 0.0f || color.y > 1.0f ||
		    color.z < 0.0f || color.z > 1.0f ||
		    intensity < 0.0f || range < 0.0f)
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
		Lights.pointLights.push_back(pointLight);
		PointLightId lightId;
		lightId.value = Lights.pointLights.size() - 1;
		return lightId;
	}

	/// @copydoc registerPointLight(const math::vec3&,const math::vec3&,float,const math::vec3&,float,float,float,float,float)
	/// @warning Declared but not yet defined — calling this overload will
	/// fail to link. Spot lights can be registered in data (lightloader.cpp
	/// has no caller for this yet either) but the actual implementation is
	/// still a TODO.
	std::expected<SpotLightId, LightRegisterError> registerPointLight(
		const math::vec3& direction,
		const math::vec3& position,
		float range,
		const math::vec3& color,
		float intensity,
		float linearAttenuation,
		float quadraticAttenuation,
		float innerCutoffAngle,
		float outerCutoffAngle
	);

} // namespace clz::renderer