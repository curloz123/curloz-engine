/**
 * @file mainpipeline.frag
 * @author curl0z
 * @brief main pipeline's fragment shader
 */
#version 460

/// In's
layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inFragWPos;
layout(location = 3) in vec3 inCameraPos;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

/// Uniform's
layout(push_constant) uniform PushConstants
{
	mat4 model;

	vec4 baseColorFactor;
	uint baseTextureIndex;
	float metallicFactor;
	float roughnessFactor;
	uint metallic_roughnessTextureIndex;
	uint normalTextureIndex;
} PC;

// Textures
#define NULL_TEXTURE uint(-1)
layout(set = 1, binding = 0) uniform sampler2D textures[256];

// light data
layout (set = 2, binding = 0)
uniform LightsDataUBO
{
	uint numPointLights;
	uint numSpotLights;
} u_LightsData;

// directional light
struct DirectionalLight
{
	// xyz dir, w pad
	vec4 direction;

	/// xyz is rgb, w is intensity
	vec4 color;
};
layout(std140, set = 2, binding = 1)
uniform  DirectionalLightUBO
{
	DirectionalLight dirLight;
} u_DirLight;

// point light
struct PointLight
{
	vec3 position;
	float range;

	vec3 color;
	float intensity;

	/// xyz is kc, kl, kd. w is padding
	vec4 attenuation;
};
layout(std430, set = 2, binding = 2) readonly buffer PointLightSSBO
{
	PointLight pointLights[];
} s_PointLights;

/// Out's
layout(location = 0) out vec4 outColor;

/// functions
#define PI 3.14159265359
vec3 calcDirLight(
		const vec4 base,
		const vec3 viewDir,
		const vec3 normal,
		const float metallic,
		const float roughness);
vec3 calcPointLight(
		const PointLight light,
		const vec3 base,
		const vec3 viewDir,
		const vec3 normal,
		const float metallic,
		const float roughness);

float distributionGGX(
		const vec3 normal,
		const vec3 halfway,
		const float roughness);
float geometrySchlickGGX(
		const float NdotV,
		const float roughness);
float geometrySmith(
		const vec3 normal,
		const vec3 view,
		const vec3 lightPos,
		const float roughness);

vec3 fresnelSchlick(
		const float cosTheta,
		const vec3 surfaceReflection0Incidence);

void main()
{
	vec3 normal;
	if (PC.normalTextureIndex == NULL_TEXTURE)
	{
		normal = inNormal;
	}
	else
	{
		mat3 TBN = mat3(inTangent, inBitangent, inNormal);
		vec3 normal = texture(textures[PC.normalTextureIndex], inUV).rgb;
		normal = (normal * 2.0) - vec3(1.0);
		normal = normalize(TBN * normal);
	}

	const vec3 viewDir = normalize(inCameraPos.xyz - inFragWPos);

	vec4 base;
	if (PC.baseTextureIndex == NULL_TEXTURE)
	{
		base = PC.baseColorFactor;
	}
	else
	{
		base =
			texture(textures[PC.baseTextureIndex], inUV) *
			PC.baseColorFactor;
	}

	float metallic;
	float roughness;

	if (PC.metallic_roughnessTextureIndex == NULL_TEXTURE)
	{
		metallic = clamp(PC.metallicFactor, 0.0, 1.0);
		roughness = clamp(PC.roughnessFactor, 0.0, 1.0);
	}
	else
	{
		metallic =
			texture(textures[PC.metallic_roughnessTextureIndex], inUV).b *
			PC.metallicFactor;
		roughness =
			texture(textures[PC.metallic_roughnessTextureIndex], inUV).g *
			PC.roughnessFactor;
	}


	vec3 L0 = vec3(0.0);

	L0 += calcDirLight(
				base,
				viewDir,
				normal,
				metallic,
				roughness);

	for (int i = 0; i < u_LightsData.numPointLights; ++i)
	{
		L0 += calcPointLight(
					s_PointLights.pointLights[i],
					base.xyz,
					viewDir,
					normal,
					metallic,
					roughness);
		/*
		PointLight light = s_PointLights.pointLights[i];
		float distance = max(length(light.position - inFragWPos), 0.05);
		/*
		if (distance > light.range)
			continue;

		float attenuation = 1.0 / (
			light.attenuation.x +
			light.attenuation.y * distance +
			light.attenuation.z * (distance * distance)
		);
		vec3 lightDir = normalize(light.position - inFragWPos);
		vec3 halfway = normalize(viewDir + lightDir);
		vec3 radiance = light.color * light.intensity * attenuation;

		vec3 F0 = vec3(0.04);
		F0 = mix(F0, base.rgb, metallic);
		vec3 F = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);
		float NDF = distributionGGX(normal, halfway, roughness);
		float G = geometrySmith(normal, viewDir, lightDir, roughness);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		float NdotL = max(dot(normal, lightDir), 0.0);
		L0 += ((kD * base.rgb / PI) + specular) * radiance * NdotL;
		*/
	}

	vec3 ambient = vec3(0.03) * base.rgb;

	vec3 color = ambient + L0;

	outColor = vec4(color, 1.0);
}

vec3 calcDirLight(
		const vec4 base,
		const vec3 viewDir,
		const vec3 normal,
		const float metallic,
		const float roughness)
{
	DirectionalLight light = u_DirLight.dirLight;
	vec3 halfway = normalize(viewDir + light.direction.xyz);
	vec3 radiance = light.color.xyz * light.color.xyz * light.color.w;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, base.rgb, metallic);
	vec3 F = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);
	float NDF = distributionGGX(normal, halfway, roughness);
	float G = geometrySmith(normal, viewDir, light.direction.xyz, roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, light.direction.xyz), 0.0) + 0.0001;
	vec3 specular = numerator / denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	float NdotL = max(dot(normal, light.direction.xyz), 0.0);
	return ((kD * base.rgb / PI) + specular) * radiance * NdotL;
}

vec3 calcPointLight(
		const PointLight light,
		const vec3 base,
		const vec3 viewDir,
		const vec3 normal,
		const float metallic,
		const float roughness)
{
	float distance = max(length(light.position - inFragWPos), 0.05);
    if (distance > light.range)
        return vec3(0.0);

	float attenuation = 1.0 / (
		light.attenuation.x +
		light.attenuation.y * distance +
		light.attenuation.z * (distance * distance)
	);
	vec3 lightDir = normalize(light.position - inFragWPos);
	vec3 halfway = normalize(viewDir + lightDir);
	vec3 radiance = light.color * light.intensity * attenuation;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, base.rgb, metallic);
	vec3 F = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);
	float NDF = distributionGGX(normal, halfway, roughness);
	float G = geometrySmith(normal, viewDir, lightDir, roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
	vec3 specular = numerator / denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	float NdotL = max(dot(normal, lightDir), 0.0);
	return ((kD * base.rgb / PI) + specular) * radiance * NdotL;
}

float distributionGGX(const vec3 normal, const vec3 halfway, const float roughness)
{
	const float alphaSquare = pow(max(roughness, 0.001), 4);
	const float NdotH_Square = pow(max(dot(normal, halfway), 0.0), 2.0);
	const float denom = PI * pow((NdotH_Square * (alphaSquare - 1) + 1), 2);

	return alphaSquare / denom;
}
float geometrySchlickGGX(const float NdotFoo, const float roughness)
{
	const float k = pow((roughness + 1), 2) / 8.0;
	return NdotFoo / ((NdotFoo * (1 - k)) + k);
}
float geometrySmith(const vec3 normal, const vec3 view, const vec3 lightPos, const float roughness)
{
	const float NdotV = max(dot(normal, view), 0.0);
	const float NdotL = max(dot(normal, lightPos), 0.0);
	const float GGX1 = geometrySchlickGGX(NdotV, roughness);
	const float GGX2 = geometrySchlickGGX(NdotL, roughness);

	return GGX1 * GGX2;
}
vec3 fresnelSchlick(const float cosTheta, const vec3 surfaceReflection0Incidence)
{
	return surfaceReflection0Incidence +
		((vec3(1.0) - surfaceReflection0Incidence) * (pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0)));
}
