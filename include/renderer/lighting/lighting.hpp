/**
 * @file lighting.hpp
 * @author curl0z
 * @brief Engine-facing light registry and accessors.
 *
 * Owns the CPU-side source of truth for all lights (Lights global) and
 * exposes typed handles (DirectionalLightId/PointLightId/SpotLightId)
 * plus getter/setter pairs so the rest of the engine never touches
 * the raw arrays directly.
 *
 * @note Everything except the editor should use the getters below —
 * writing directly into Lights bypasses validation and id bookkeeping.
 */

#pragma once

#include "lights.hpp"
#include "math/vec3.hpp"
#include <expected>
#include <vector>

namespace clz::renderer
{

	/// @brief Opaque handle to a registered directional light.
	/// @note Only ever has value 0 — engine supports a single sun.
	struct DirectionalLightId
	{
		uint32_t value;
	};

	/// @brief Opaque handle to a registered point light.
	/// Index into Lights.pointLights.
	struct PointLightId
	{
		uint32_t value;
	};

	/// @brief Opaque handle to a registered spot light.
	/// Index into Lights.spotLights.
	struct SpotLightId
	{
		uint32_t value;
	};

	/// @brief Hard cap on directional lights (engine only models one sun).
	inline constexpr uint8_t MAX_DIR_LIGHTS = 1;
	/// @brief Hard cap on point lights, must match shader-side array sizing.
	inline constexpr uint8_t MAX_POINT_LIGHTS = 255;
	/// @brief Hard cap on spot lights, must match shader-side array sizing.
	inline constexpr uint8_t MAX_SPOT_LIGHTS = 255;

	/**
	 * @brief CPU-side storage for every light in the scene.
	 *
	 * Indices into these vectors are what DirectionalLightId/PointLightId/
	 * SpotLightId::value refer to. This is memcpy'd (or field-copied) into
	 * the corresponding UBO/SSBO each frame in updateLightDescriptor.
	 */
	struct LightArrays
	{
		std::vector<DirectionalLight> directionalLight;
		std::vector<PointLight> pointLights;
		std::vector<SpotLight> spotLights;
	};
	/// @brief Global light registry. See LightArrays.
	inline LightArrays Lights;

	/// @brief Reasons a register*Light call can fail.
	enum class LightRegisterError
	{
		LIMIT_EXCEEDED,	  ///< MAX_*_LIGHTS would be exceeded by this registration.
		INVALID_PARAMETER ///< A parameter (color/intensity/range/etc.) is out of range.
	};

	/// @brief Creates directional light
	/// @param direction Direction of light
	/// @param color Color of light
	/// @param intensity Intensity of light
	/// @return directional light Id (always 0),
	/// else LightRegisterError as error if something went wrong
	/// @note right now engine only supports single directional light (the sun)
	/// so id.value will always return 0.
	std::expected<DirectionalLightId, LightRegisterError> registerDirectionalLight(
		const math::vec3& direction,
		const math::vec3& color,
		float intensity
	);

	/**
	 * @brief Registers a new point light.
	 * @param position World-space position of the light.
	 * @param range Maximum distance the light can affect a fragment.
	 * @param color RGB color, each channel expected in [0, 1].
	 * @param intensity Light intensity/brightness multiplier.
	 * @param linearAttenuation Linear falloff term (Kl in the attenuation formula).
	 * @param quadraticAttenuation Quadratic falloff term (Kq in the attenuation formula).
	 * @return A valid PointLightId on success, else a LightRegisterError.
	 * @note Constant attenuation term (Kc) is always fixed at 1.0 — not
	 * configurable through this API.
	 */
	std::expected<PointLightId, LightRegisterError> registerPointLight(
		const math::vec3& position,
		float range,
		const math::vec3& color,
		float intensity,
		float linearAttenuation,
		float quadraticAttenuation
	);

	/**
	 * @brief Registers a new spot light.
	 * @param direction Direction the cone points in.
	 * @param position World-space position of the light.
	 * @param range Maximum distance the light can affect a fragment.
	 * @param color RGB color, each channel expected in [0, 1].
	 * @param intensity Light intensity/brightness multiplier.
	 * @param linearAttenuation Linear falloff term (Kl in the attenuation formula).
	 * @param quadraticAttenuation Quadratic falloff term (Kq in the attenuation formula).
	 * @param innerCutoffAngle Angle (degrees) within which light is at full strength.
	 * @param outerCutoffAngle Angle (degrees) beyond which light is fully attenuated;
	 * the region between inner and outer produces the soft cone edge.
	 * @return A valid SpotLightId on success, else a LightRegisterError.
	 * @warning Despite the name, this is the spot light overload
	 * (resolved by parameter list) — consider renaming to
	 * registerSpotLight to avoid confusion/accidental overload resolution
	 * bugs with the point light version above.
	 */
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

	/// @brief Gets the direction of a directional light.
	/// @param id Handle returned by registerDirectionalLight.
	/// @return Normalized direction vector.
	inline math::vec3 getDirLightDirection(const DirectionalLightId id)
	{
		const auto& l = Lights.directionalLight[id.value];
		return {l.direction.x, l.direction.y, l.direction.z};
	}
	/// @brief Sets the direction of a directional light.
	/// @param id Handle returned by registerDirectionalLight.
	/// @param dir New direction; normalized internally before storing.
	inline void setDirLightDirection(const DirectionalLightId id, const math::vec3& dir)
	{
		const math::vec3 n = math::normalize(dir);
		auto& l = Lights.directionalLight[id.value];
		l.direction.x = n.x;
		l.direction.y = n.y;
		l.direction.z = n.z;
	}

	/// @brief Gets the RGB color of a directional light.
	/// @param id Handle returned by registerDirectionalLight.
	inline math::vec3 getDirLightColor(const DirectionalLightId id)
	{
		const auto& l = Lights.directionalLight[id.value];
		return {l.color.x, l.color.y, l.color.z};
	}
	/// @brief Sets the RGB color of a directional light.
	/// @param id Handle returned by registerDirectionalLight.
	/// @param color New RGB color. Intensity (color.w) is left untouched.
	inline void setDirLightColor(const DirectionalLightId id, const math::vec3& color)
	{
		auto& l = Lights.directionalLight[id.value];
		l.color.x = color.x;
		l.color.y = color.y;
		l.color.z = color.z;
	}

	/// @brief Gets the intensity of a directional light (stored in color.w).
	/// @param id Handle returned by registerDirectionalLight.
	inline float getDirLightIntensity(const DirectionalLightId id)
	{
		return Lights.directionalLight[id.value].color.w;
	}
	/// @brief Sets the intensity of a directional light (stored in color.w).
	/// @param id Handle returned by registerDirectionalLight.
	/// @param intensity New intensity value.
	inline void setDirLightIntensity(const DirectionalLightId id, const float intensity)
	{
		Lights.directionalLight[id.value].color.w = intensity;
	}

	/// @brief Gets the world-space position of a point light.
	/// @param id Handle returned by registerPointLight.
	inline math::vec3 getPointLightPosition(const PointLightId id)
	{
		const auto& l = Lights.pointLights[id.value];
		return {l.position.x, l.position.y, l.position.z};
	}
	/// @brief Sets the world-space position of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param position New world-space position.
	inline void setPointLightPosition(const PointLightId id, const math::vec3& position)
	{
		auto& l = Lights.pointLights[id.value];
		l.position.x = position.x;
		l.position.y = position.y;
		l.position.z = position.z;
	}

	/// @brief Gets the maximum effective range of a point light.
	/// @param id Handle returned by registerPointLight.
	inline float getPointLightRange(const PointLightId id)
	{
		return Lights.pointLights[id.value].range;
	}
	/// @brief Sets the maximum effective range of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param range New range; fragments farther than this are skipped
	/// entirely in the lighting loop (see mainpipeline.frag).
	inline void setPointLightRange(const PointLightId id, const float range)
	{
		Lights.pointLights[id.value].range = range;
	}

	/// @brief Gets the RGB color of a point light.
	/// @param id Handle returned by registerPointLight.
	inline math::vec3 getPointLightColor(const PointLightId id)
	{
		const auto& l = Lights.pointLights[id.value];
		return {l.color.x, l.color.y, l.color.z};
	}
	/// @brief Sets the RGB color of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param color New RGB color.
	inline void setPointLightColor(const PointLightId id, const math::vec3& color)
	{
		auto& l = Lights.pointLights[id.value];
		l.color.x = color.x;
		l.color.y = color.y;
		l.color.z = color.z;
	}

	/// @brief Gets the intensity of a point light.
	/// @param id Handle returned by registerPointLight.
	inline float getPointLightIntensity(const PointLightId id)
	{
		return Lights.pointLights[id.value].intensity;
	}
	/// @brief Sets the intensity of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param intensity New intensity value.
	inline void setPointLightIntensity(const PointLightId id, const float intensity)
	{
		Lights.pointLights[id.value].intensity = intensity;
	}

	/// @brief Gets the linear attenuation term (Kl) of a point light.
	/// @param id Handle returned by registerPointLight.
	inline float getPointLightLinearAttenuation(const PointLightId id)
	{
		return Lights.pointLights[id.value].attenuation.y;
	}
	/// @brief Sets the linear attenuation term (Kl) of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param linear New linear attenuation coefficient.
	inline void setPointLightLinearAttenuation(const PointLightId id, const float linear)
	{
		Lights.pointLights[id.value].attenuation.y = linear;
	}

	/// @brief Gets the quadratic attenuation term (Kq) of a point light.
	/// @param id Handle returned by registerPointLight.
	inline float getPointLightQuadraticAttenuation(const PointLightId id)
	{
		return Lights.pointLights[id.value].attenuation.z;
	}
	/// @brief Sets the quadratic attenuation term (Kq) of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param quadratic New quadratic attenuation coefficient.
	inline void setPointLightQuadraticAttenuation(const PointLightId id, const float quadratic)
	{
		Lights.pointLights[id.value].attenuation.z = quadratic;
	}

	/// @brief Gets the direction of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline math::vec3 getSpotLightDirection(const SpotLightId id)
	{
		const auto& l = Lights.spotLights[id.value];
		return {l.direction.x, l.direction.y, l.direction.z};
	}
	/// @brief Sets the direction of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param dir New direction; normalized internally before storing.
	inline void setSpotLightDirection(const SpotLightId id, const math::vec3& dir)
	{
		const math::vec3 n = math::normalize(dir);
		auto& l = Lights.spotLights[id.value];
		l.direction.x = n.x;
		l.direction.y = n.y;
		l.direction.z = n.z;
	}

	/// @brief Gets the world-space position of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline math::vec3 getSpotLightPosition(const SpotLightId id)
	{
		const auto& l = Lights.spotLights[id.value];
		return math::vec3(l.position.x, l.position.y, l.position.z);
	}
	/// @brief Sets the world-space position of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param position New world-space position.
	inline void setSpotLightPosition(const SpotLightId id, const math::vec3& position)
	{
		auto& l = Lights.spotLights[id.value];
		l.position.x = position.x;
		l.position.y = position.y;
		l.position.z = position.z;
	}

	/// @brief Gets the maximum effective range of a spot light (stored in position.w).
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightRange(const SpotLightId id)
	{
		return Lights.spotLights[id.value].position.w;
	}
	/// @brief Sets the maximum effective range of a spot light (stored in position.w).
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param range New range.
	inline void setSpotLightRange(const SpotLightId id, const float range)
	{
		Lights.spotLights[id.value].position.w = range;
	}

	/// @brief Gets the RGB color of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline math::vec3 getSpotLightColor(const SpotLightId id)
	{
		const auto& l = Lights.spotLights[id.value];
		return math::vec3(l.color.x, l.color.y, l.color.z);
	}
	/// @brief Sets the RGB color of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param color New RGB color.
	inline void setSpotLightColor(const SpotLightId id, const math::vec3& color)
	{
		auto& l = Lights.spotLights[id.value];
		l.color.x = color.x;
		l.color.y = color.y;
		l.color.z = color.z;
	}

	/// @brief Gets the intensity of a spot light (stored in color.w).
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightIntensity(const SpotLightId id)
	{
		return Lights.spotLights[id.value].color.w;
	}
	/// @brief Sets the intensity of a spot light (stored in color.w).
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param intensity New intensity value.
	inline void setSpotLightIntensity(const SpotLightId id, const float intensity)
	{
		Lights.spotLights[id.value].color.w = intensity;
	}

	/// @brief Gets the linear attenuation term (Kl) of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightLinearAttenuation(const SpotLightId id)
	{
		return Lights.spotLights[id.value].attenuation.y;
	}
	/// @brief Sets the linear attenuation term (Kl) of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param linear New linear attenuation coefficient.
	inline void setSpotLightLinearAttenuation(const SpotLightId id, const float linear)
	{
		Lights.spotLights[id.value].attenuation.y = linear;
	}

	/// @brief Gets the quadratic attenuation term (Kq) of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightQuadraticAttenuation(const SpotLightId id)
	{
		return Lights.spotLights[id.value].attenuation.z;
	}
	/// @brief Sets the quadratic attenuation term (Kq) of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param quadratic New quadratic attenuation coefficient.
	inline void setSpotLightQuadraticAttenuation(const SpotLightId id, const float quadratic)
	{
		Lights.spotLights[id.value].attenuation.z = quadratic;
	}

	/// @brief Gets the inner cutoff angle (degrees) of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightInnerCutoff(const SpotLightId id)
	{
		return Lights.spotLights[id.value].cutoff.x;
	}
	/// @brief Sets the inner cutoff angle (degrees) of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param angle New inner cutoff angle, in degrees.
	inline void setSpotLightInnerCutoff(const SpotLightId id, const float angle)
	{
		Lights.spotLights[id.value].cutoff.x = angle;
	}

	/// @brief Gets the outer cutoff angle (degrees) of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightOuterCutoff(const SpotLightId id)
	{
		return Lights.spotLights[id.value].cutoff.y;
	}
	/// @brief Sets the outer cutoff angle (degrees) of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param angle New outer cutoff angle, in degrees. Must be >= inner
	/// cutoff for the soft edge to make sense.
	inline void setSpotLightOuterCutoff(const SpotLightId id, const float angle)
	{
		Lights.spotLights[id.value].cutoff.y = angle;
	}

} // namespace clz::renderer