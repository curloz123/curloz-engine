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
 * @note Everything except the editor should use the getters below,
 * writing directly into Lights bypasses validation and id bookkeeping.
 */

#pragma once

#include "core/id_interface.hpp"
#include "lights.hpp"
#include "math/vec3.hpp"
#include <expected>
#include <cstdint>
#include <array>
#include "core/logs.hpp"

namespace clz::renderer
{

	/// @brief Opaque handle to a registered directional light.
	/// @note Only ever has value 0, engine supports a single sun.
	class DirectionalLightId : public IdInterface<uint32_t>{};

	/// @brief Opaque handle to a registered point light.
	/// Index into Lights.pointLights.
	class PointLightId : public IdInterface<uint32_t>{};

	/// @brief Opaque handle to a registered spot-light.
	/// Index into Lights.spotLights.
	class SpotLightId : public IdInterface<uint32_t>{};

	/// @brief Hard cap on directional lights (engine only models one sun).
	inline constexpr uint8_t MAX_DIR_LIGHTS = 1;
	/// @brief Hard cap on point lights, must match shader-side array sizing.
	/// Max 256 lights
	inline constexpr uint8_t MAX_POINT_LIGHTS = UINT8_MAX;
	/// @brief Hard cap on spot-lights, must match shader-side array sizing.
	/// Max 256 lights
	inline constexpr uint8_t MAX_SPOT_LIGHTS = UINT8_MAX;

	/**
	 * @brief CPU-side storage for every light in the scene.
	 *
	 * Indices into these vectors are what DirectionalLightId/PointLightId/
	 * SpotLightId::value refer to. This is memcpy'd (or field-copied) into
	 * the corresponding UBO/SSBO each frame in updateLightDescriptor.
	 */
	struct LightArrays
	{
		std::array<DirectionalLight, MAX_DIR_LIGHTS> directionalLight;
		std::array<PointLight, MAX_POINT_LIGHTS> pointLights;
		std::array<SpotLight, MAX_SPOT_LIGHTS> spotLights;
	};
	/// @brief Global light registry. See LightArrays.
	inline LightArrays Lights;

	inline uint8_t numDirectionalLights = 0;
	inline uint8_t numPointLights = 0;
	inline uint8_t numSpotLights = 0;

	/// @brief Reasons a register*Light call can fail.
	enum class LightRegisterError
	{
		///< MAX_*_LIGHTS would be exceeded by this registration.
		LIMIT_EXCEEDED,
		///< A parameter (color/intensity/range/etc.) is out of range.
		INVALID_PARAMETER
	};

	/// @brief Creates directional light
	/// @param direction Direction of light
	/// @param color Color of light
	/// @param intensity Intensity of light
	/// @return directional light Id (always 0),
	/// else LightRegisterError as error if something went wrong
	/// @note right now engine only supports single directional light (the sun)
	/// so id.getId() will always return 0.
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
	 * @brief Registers a new spot-light.
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
	 */
	std::expected<SpotLightId, LightRegisterError> registerSpotLight(
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
		if (id.getId() >= numDirectionalLights) [[unlikely]]
		{
			clz::log::error("Invalid dir light id passed to query direction");
			return math::vec3(1.0);
		}
		const auto& l = Lights.directionalLight[id.getId()];
		return {l.direction.x, l.direction.y, l.direction.z};
	}
	/// @brief Sets the direction of a directional light.
	/// @param id Handle returned by registerDirectionalLight.
	/// @param dir New direction; normalized internally before storing.
	inline void setDirLightDirection(const DirectionalLightId id, const math::vec3& dir)
	{
		if (id.getId() >= numDirectionalLights) [[unlikely]]
		{
			clz::log::error("Invalid dir light id passed to set direction");
			return;
		}

		const math::vec3 n = math::normalize(dir);
		auto& l = Lights.directionalLight[id.getId()];
		l.direction.x = n.x;
		l.direction.y = n.y;
		l.direction.z = n.z;
	}

	/// @brief Gets the RGB color of a directional light.
	/// @param id Handle returned by registerDirectionalLight.
	inline math::vec3 getDirLightColor(const DirectionalLightId id)
	{
		if (id.getId() >= numDirectionalLights) [[unlikely]]
		{
			clz::log::error("Invalid dir light id passed to query color");
			return math::vec3(1.0f);
		}

		const auto& l = Lights.directionalLight[id.getId()];
		return {l.color.x, l.color.y, l.color.z};
	}
	/// @brief Sets the RGB color of a directional light.
	/// @param id Handle returned by registerDirectionalLight.
	/// @param color New RGB color. Intensity (color.w) is left untouched.
	inline void setDirLightColor(const DirectionalLightId id, const math::vec3& color)
	{
		if (id.getId() >= numDirectionalLights) [[unlikely]]
		{
			clz::log::error("Invalid dir light id passed to set color");
			return;
		}
		auto& l = Lights.directionalLight[id.getId()];
		l.color.x = color.x;
		l.color.y = color.y;
		l.color.z = color.z;
	}

	/// @brief Gets the intensity of a directional light (stored in color.w).
	/// @param id Handle returned by registerDirectionalLight.
	inline float getDirLightIntensity(const DirectionalLightId id)
	{
		if (id.getId() >= numDirectionalLights) [[unlikely]]
		{
			clz::log::error("Invalid dir light id passed to query intensity");
			return 1.0f;
		}

		return Lights.directionalLight[id.getId()].intensity;
	}

	/// @brief Sets the intensity of a directional light (stored in color.w).
	/// @param id Handle returned by registerDirectionalLight.
	/// @param intensity New intensity value.
	inline void setDirLightIntensity(const DirectionalLightId id, const float intensity)
	{
		if (id.getId() >= numDirectionalLights) [[unlikely]]
		{
			clz::log::error("Invalid dir light id passed to set intensity");
			return;
		}

		Lights.directionalLight[id.getId()].intensity = intensity;
	}

	/// @brief Gets the world-space position of a point light.
	/// @param id Handle returned by registerPointLight.
	inline math::vec3 getPointLightPosition(const PointLightId id)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to query position");
			return math::vec3(1.0f);
		}
		const auto& l = Lights.pointLights[id.getId()];
		return {l.position.x, l.position.y, l.position.z};
	}
	/// @brief Sets the world-space position of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param position New world-space position.
	inline void setPointLightPosition(const PointLightId id, const math::vec3& position)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to set position");
			return;
		}
		auto& l = Lights.pointLights[id.getId()];
		l.position.x = position.x;
		l.position.y = position.y;
		l.position.z = position.z;
	}

	/// @brief Gets the maximum effective range of a point light.
	/// @param id Handle returned by registerPointLight.
	inline float getPointLightRange(const PointLightId id)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to get range");
			return 10.0f;
		}

		return Lights.pointLights[id.getId()].range;
	}
	/// @brief Sets the maximum effective range of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param range New range; fragments farther than this are skipped
	/// entirely in the lighting loop (see mainpipeline.frag).
	inline void setPointLightRange(const PointLightId id, const float range)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to set range");
			return;
		}
		Lights.pointLights[id.getId()].range = range;
	}

	/// @brief Gets the RGB color of a point light.
	/// @param id Handle returned by registerPointLight.
	inline math::vec3 getPointLightColor(const PointLightId id)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to get color");
			return math::vec3(1.0f);
		}

		const auto& l = Lights.pointLights[id.getId()];
		return {l.color.x, l.color.y, l.color.z};
	}
	/// @brief Sets the RGB color of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param color New RGB color.
	inline void setPointLightColor(const PointLightId id, const math::vec3& color)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to set color");
			return;
		}

		auto& l = Lights.pointLights[id.getId()];
		l.color.x = color.x;
		l.color.y = color.y;
		l.color.z = color.z;
	}

	/// @brief Gets the intensity of a point light.
	/// @param id Handle returned by registerPointLight.
	inline float getPointLightIntensity(const PointLightId id)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to get intensity");
			return 1.0f;
		}

		return Lights.pointLights[id.getId()].intensity;
	}
	/// @brief Sets the intensity of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param intensity New intensity value.
	inline void setPointLightIntensity(const PointLightId id, const float intensity)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to set intensity");
			return;
		}

		Lights.pointLights[id.getId()].intensity = intensity;
	}

	/// @brief Gets the linear attenuation term (Kl) of a point light.
	/// @param id Handle returned by registerPointLight.
	inline float getPointLightLinearAttenuation(const PointLightId id)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to get attenuation");
			return 0.09f;
		}

		return Lights.pointLights[id.getId()].attenuation.y;
	}
	/// @brief Sets the linear attenuation term (Kl) of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param linear New linear attenuation coefficient.
	inline void setPointLightLinearAttenuation(const PointLightId id, const float linear)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to set attenuation");
			return;
		}

		Lights.pointLights[id.getId()].attenuation.y = linear;
	}

	/// @brief Gets the quadratic attenuation term (Kq) of a point light.
	/// @param id Handle returned by registerPointLight.
	inline float getPointLightQuadraticAttenuation(const PointLightId id)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to get attenuation");
			return 0.09f;
		}

		return Lights.pointLights[id.getId()].attenuation.z;
	}
	/// @brief Sets the quadratic attenuation term (Kq) of a point light.
	/// @param id Handle returned by registerPointLight.
	/// @param quadratic New quadratic attenuation coefficient.
	inline void setPointLightQuadraticAttenuation(const PointLightId id, const float quadratic)
	{
		if (id.getId() >= numPointLights) [[unlikely]]
		{
			clz::log::error("Invalid point light id passed to set attenuation");
			return;
		}

		Lights.pointLights[id.getId()].attenuation.z = quadratic;
	}

	/// @brief Gets the direction of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline math::vec3 getSpotLightDirection(const SpotLightId id)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to get direction");
			return math::vec3(1.0f);
		}
		const auto& l = Lights.spotLights[id.getId()];
		return {l.direction.x, l.direction.y, l.direction.z};
	}
	/// @brief Sets the direction of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param dir New direction; normalized internally before storing.
	inline void setSpotLightDirection(const SpotLightId id, const math::vec3& dir)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to set direction");
			return;
		}

		const math::vec3 n = math::normalize(dir);
		auto& l = Lights.spotLights[id.getId()];
		l.direction.x = n.x;
		l.direction.y = n.y;
		l.direction.z = n.z;
	}

	/// @brief Gets the world-space position of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline math::vec3 getSpotLightPosition(const SpotLightId id)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to get position");
			return math::vec3(1.0f);
		}

		const auto& l = Lights.spotLights[id.getId()];
		return math::vec3(l.position.x, l.position.y, l.position.z);
	}
	/// @brief Sets the world-space position of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param position New world-space position.
	inline void setSpotLightPosition(const SpotLightId id, const math::vec3& position)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to set position");
			return;
		}

		auto& l = Lights.spotLights[id.getId()];
		l.position.x = position.x;
		l.position.y = position.y;
		l.position.z = position.z;
	}

	/// @brief Gets the maximum effective range of a spot light (stored in position.w).
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightRange(const SpotLightId id)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to get range");
			return 0.0f;
		}

		return Lights.spotLights[id.getId()].range;
	}
	/// @brief Sets the maximum effective range of a spot light (stored in position.w).
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param range New range.
	inline void setSpotLightRange(const SpotLightId id, const float range)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to set range");
			return;
		}

		Lights.spotLights[id.getId()].range = range;
	}

	/// @brief Gets the RGB color of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline math::vec3 getSpotLightColor(const SpotLightId id)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to get color");
			return math::vec3(1.0f);
		}

		const auto& l = Lights.spotLights[id.getId()];
		return math::vec3(l.color.x, l.color.y, l.color.z);
	}
	/// @brief Sets the RGB color of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param color New RGB color.
	inline void setSpotLightColor(const SpotLightId id, const math::vec3& color)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to set color");
			return;
		}

		auto& l = Lights.spotLights[id.getId()];
		l.color.x = color.x;
		l.color.y = color.y;
		l.color.z = color.z;
	}

	/// @brief Gets the intensity of a spot light (stored in color.w).
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightIntensity(const SpotLightId id)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to get intensity");
			return 0.0f;
		}
		return Lights.spotLights[id.getId()].intensity;
	}
	/// @brief Sets the intensity of a spot light (stored in color.w).
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param intensity New intensity value.
	inline void setSpotLightIntensity(const SpotLightId id, const float intensity)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to set intensity");
			return;
		}
		Lights.spotLights[id.getId()].intensity = intensity;
	}

	/// @brief Gets the linear attenuation term (Kl) of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightLinearAttenuation(const SpotLightId id)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to get attenuation");
			return 0.0f;
		}
		return Lights.spotLights[id.getId()].attenuation.x;
	}
	/// @brief Sets the linear attenuation term (Kl) of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param linear New linear attenuation coefficient.
	inline void setSpotLightLinearAttenuation(const SpotLightId id, const float linear)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to set attenuation");
			return;
		}
		Lights.spotLights[id.getId()].attenuation.x = linear;
	}

	/// @brief Gets the quadratic attenuation term (Kq) of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightQuadraticAttenuation(const SpotLightId id)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to get attenuation");
			return 0.0f;
		}

		return Lights.spotLights[id.getId()].attenuation.y;
	}
	/// @brief Sets the quadratic attenuation term (Kq) of a spot light.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param quadratic New quadratic attenuation coefficient.
	inline void setSpotLightQuadraticAttenuation(const SpotLightId id, const float quadratic)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to set attenuation");
			return;
		}
		Lights.spotLights[id.getId()].attenuation.y = quadratic;
	}

	/// @brief Gets the inner cutoff angle (degrees) of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightInnerCutoff(const SpotLightId id)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to get inner cutoff");
			return 0.0f;
		}
		return Lights.spotLights[id.getId()].cutoff.x;
	}
	/// @brief Sets the inner cutoff angle (degrees) of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param angle New inner cutoff angle, in degrees.
	inline void setSpotLightInnerCutoff(const SpotLightId id, const float angle)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to set inner cutoff");
			return;
		}

		Lights.spotLights[id.getId()].cutoff.x = angle;
	}

	/// @brief Gets the outer cutoff angle (degrees) of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	inline float getSpotLightOuterCutoff(const SpotLightId id)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to get outer cutoff");
			return 0.0f;
		}

		return Lights.spotLights[id.getId()].cutoff.y;
	}
	/// @brief Sets the outer cutoff angle (degrees) of a spot light's cone.
	/// @param id Handle returned by registerPointLight (spot overload).
	/// @param angle New outer cutoff angle, in degrees. Must be >= inner
	/// cutoff for the soft edge to make sense.
	inline void setSpotLightOuterCutoff(const SpotLightId id, const float angle)
	{
		if (id.getId() >= numSpotLights) [[unlikely]]
		{
			clz::log::error("Invalid spot light id passed to set outer cutoff");
			return;
		}

		Lights.spotLights[id.getId()].cutoff.y = angle;
	}

} // namespace clz::renderer
