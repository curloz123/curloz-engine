#pragma once

#include "lights.hpp"
#include <cstdint>
#include <vector>
#include "math/vec3.hpp"
#include <vulkan/vulkan.h>

namespace clz::renderer
{

struct DirectionalLightId
{
	uint32_t value;
};

struct PointLightId
{
	uint32_t value;
};

struct SpotLightId
{
	uint32_t value;
};

inline constexpr uint8_t MAX_LIGHTS = 255;
struct LightMemory
{
	VkBuffer DirectionalLightBuffer;

};
struct LightArrays
{
	DirectionalLight directionalLight = {};
	std::vector<PointLight> pointLights;
	std::vector<VkDeviceSize> pointLightOffsets;
	std::vector<SpotLight> spotLights;
};
inline LightArrays Lights;

/// @brief Creates directional light
/// @param direction Direction of light
/// @param color Color of light
/// @param intensity Intensity of light
/// @note right now engine only supports single directional light (the sun)
/// so id.value will always return 0.
DirectionalLightId registerDirectionalLight(
	const math::vec3& direction,
	const math::vec3& color,
	float intensity);

PointLightId registerPointLight(
	const math::vec3& position,
	float range,
	const math::vec3& color,
	float intensity,
	float linearAttenuation,
	float quadraticAttenuation);

SpotLightId registerSpotLight(
	const math::vec3& direction,
	const math::vec3& position,
	float range,
	const math::vec3& color,
	float intensity,
	float linearAttenuation,
	float quadraticAttenuation,
	float innerCutoffAngle,
	float outerCutoffAngle);

bool createLights();
}