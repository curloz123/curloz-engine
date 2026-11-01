/**
 * @file mainpipeline.frag
 * @author curl0z
 * @brief main pipeline's fragment shader
 */
#version 460

#define NULL_TEXTURE  uint(-1)
#define NULL_MATERIAL uint(-1)

/// In's
layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inFragWPos;
layout(location = 3) in vec3 inCameraPos;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;


struct Material
{
	/// @brief Base color factor
	vec4 baseFactor;
	/// @brief ID of the bound base color texture
	uint  baseTextureId;
	uint baseSamplerId;

	/// @brief ID of metallic texture
	uint metallicRoughnessTextureId;
	uint metallicRoughnessSamplerId;
	/// @brief metallic color factor
	float metallicFactor;
	/// @brief roughness color factor
	float roughnessFactor;

	/// @brief ID of emissive texture
	uint emissiveTextureId;
	/// @brief emissive color strength
	float emissiveStrength;
	/// @brief emissive factor
	vec3 emissiveFactor;
	uint emissiveSamplerId;

	/// @brief ID of normal texture
	uint normalTextureId;
	uint normalSamplerId;

	float ior;

	float specularFactor;
	vec3  specularColorFactor;
	uint  specularTextureId;
	uint  specularSamplerId;
	uint  specularColorTextureId;
	uint  specularColorSamplerId;

	float _pad;
};

// directional light
struct DirectionalLight
{
	vec3 color;
	float intensity;

	vec3 direction;
	bool enabled;
};
// point light
struct PointLight
{
	vec3 position;
	float range;

	vec3 color;
	float intensity;

	vec3 attenuation;
	bool enabled;
};

struct SpotLight
{
	vec3 direction;
	bool enabled;

	vec3 position;
	float range;

	vec3 color;
	float intensity;

	/// x is kl, y is kd
	vec2 attenuation;
	vec2 cutoff;
};

/// Uniform's
layout(push_constant) uniform PushConstants
{
	uint matrixIndex;
	uint materialIndex;
} PC;

layout(set = 1, binding = 0) uniform texture2D textures[1024];
layout(set = 1, binding = 1) uniform sampler samplers[1024];
layout(std430, set = 1, binding = 2) readonly buffer Materials
{
	Material materials[];
} s_Materials;

// light data
layout (set = 2, binding = 0) uniform LightsDataUBO
{
	uint numPointLights;
	uint numSpotLights;
} u_LightsData;

layout(std140, set = 2, binding = 1)
uniform  DirectionalLightUBO
{
	DirectionalLight dirLight;
} u_DirLight;


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
	const float roughness,
	const float ior,
	const vec3 specularColor,
	const float specular
);
vec3 calcPointLight(
	const PointLight light,
	const vec3 base,
	const vec3 viewDir,
	const vec3 normal,
	const float metallic,
	const float roughness,
	const float ior,
	const vec3 specularColor,
	const float specular
);

float distributionGGX(
	const vec3 normal,
	const vec3 halfway,
	const float roughness
);
float geometrySchlickGGX(
	const float NdotV,
	const float roughness
);
float geometrySmith(
	const vec3 normal,
	const vec3 view,
	const vec3 lightPos,
	const float roughness
);

vec3 fresnelSchlick(
	const float cosTheta,
	const vec3 surfaceReflection0Incidence,
	const vec3 surfaceReflection90Incidence
);

void main()
{
	if (PC.materialIndex == NULL_MATERIAL)
	{
		outColor = vec4(vec3(1.0), 1.0);
		return;
	}

	Material material = s_Materials.materials[PC.materialIndex];

	vec3 normal = inNormal;
	if (material.normalTextureId != NULL_TEXTURE)
	{
		mat3 TBN = mat3(inTangent, inBitangent, inNormal);
		normal = texture(
				sampler2D(
					textures[material.normalTextureId],
					samplers[material.normalSamplerId]
				),
				inUV
			).rgb;
		normal = (normal * 2.0) - vec3(1.0);
		normal = normalize(TBN * normal);
	}

	const vec3 viewDir = normalize(inCameraPos.xyz - inFragWPos);

	vec4 base;
	if (material.baseTextureId == NULL_TEXTURE)
	{
		base = material.baseFactor;
	}
	else
	{
		base = texture(
				sampler2D(
					textures[material.baseTextureId],
					samplers[material.baseSamplerId]
				),
				inUV
			) * material.baseFactor;
	}

	float metallic = clamp(material.metallicFactor, 0.0, 1.0);
	float roughness = clamp(material.roughnessFactor, 0.0, 1.0);
	if (material.metallicRoughnessTextureId != NULL_TEXTURE)
	{
		vec4 mrColor = texture(
				sampler2D(
					textures[material.metallicRoughnessTextureId],
					samplers[material.metallicRoughnessSamplerId]
				),
				inUV
			);
		metallic *= mrColor.b;
		roughness *= mrColor.g;
	}

	float specular = material.specularFactor;
	vec3 specularColor = material.specularColorFactor;
	if (material.specularColorTextureId != NULL_TEXTURE)
	{
		vec4 s = texture(
				sampler2D(
					textures[material.specularColorTextureId],
				       	samplers[material.specularColorSamplerId]
				), 
				inUV
			 );
		specularColor *= s.rgb;

		if (material.specularTextureId == material.specularColorTextureId)
		    	specular *= s.a;
		else if (material.specularTextureId != NULL_TEXTURE)
		{
			specular *= texture(
					sampler2D(
						textures[material.specularTextureId],
					      	samplers[material.specularSamplerId]
					), inUV
				    ).a;
		}
	}
	else if (material.specularTextureId != NULL_TEXTURE)
	{
	    specular *= texture(
			    	sampler2D(
					textures[material.specularTextureId],
					samplers[material.specularSamplerId]
				), inUV
			).a;
	}


	vec3 L0 = vec3(0.0);

	L0 += calcDirLight(
			base,
			viewDir,
			normal,
			metallic,
			roughness,
			material.ior,
			specularColor,
			specular
		);

	for (int i = 0; i < u_LightsData.numPointLights; ++i)
	{
		L0 += calcPointLight(
				s_PointLights.pointLights[i],
				base.xyz,
				viewDir,
				normal,
				metallic,
				roughness,
				material.ior,
				specularColor,
				specular
			);
	}

	vec3 emissiveColor = material.emissiveFactor * material.emissiveStrength;
	if (material.emissiveTextureId != NULL_TEXTURE)
	{
		emissiveColor *= texture(
					sampler2D(
						textures[material.emissiveTextureId],
						samplers[material.emissiveSamplerId]
					),
					inUV
				).rgb;
	}

	vec3 ambient = vec3(0.01) * base.rgb;
	vec3 color = ambient + L0 + emissiveColor;

	outColor = vec4(color, 1.0);
}

vec3 calcDirLight(
	const vec4 base,
	const vec3 viewDir,
	const vec3 normal,
	const float metallic,
	const float roughness,
	const float ior,
	const vec3 specularColor,
	const float specular
)
{
	DirectionalLight light = u_DirLight.dirLight;
	vec3 lightDir = -light.direction.xyz;
	vec3 halfway = normalize(viewDir + lightDir);
	vec3 radiance = light.color.xyz * light.color.w;

	float f0Base = pow((ior - 1.0) / (ior + 1.0), 2.0);
	vec3 dielectricF0 = min(f0Base * specularColor, vec3(1.0)) * specular;
	vec3 dielectricF90 = vec3(specular);
	vec3 F0 = mix(dielectricF0, base.rgb, metallic);
	vec3 F90 = mix(dielectricF90, vec3(1.0), metallic);
	vec3 F = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0, F90);
	float NDF = distributionGGX(normal, halfway, roughness);
	float G = geometrySmith(normal, viewDir, lightDir, roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
	vec3 specularBRDF = numerator / denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	float NdotL = max(dot(normal, lightDir), 0.0);
	return ((kD * base.rgb / PI) + specularBRDF) * radiance * NdotL;
}

vec3 calcPointLight(
	const PointLight light,
	const vec3 base,
	const vec3 viewDir,
	const vec3 normal,
	const float metallic,
	const float roughness,
	const float ior,
	const vec3 specularColor,
	const float specular
)
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

	float f0Base = pow((ior - 1.0) / (ior + 1.0), 2.0);
	vec3 dielectricF0 = min(f0Base * specularColor, vec3(1.0)) * specular;
	vec3 dielectricF90 = vec3(specular);
	vec3 F0 = mix(dielectricF0, base.rgb, metallic);
	vec3 F90 = mix(dielectricF90, vec3(1.0), metallic);
	vec3 F = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0, F90);

	float NDF = distributionGGX(normal, halfway, roughness);
	float G = geometrySmith(normal, viewDir, lightDir, roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
	vec3 specularBRDF = numerator / denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	float NdotL = max(dot(normal, lightDir), 0.0);
	return ((kD * base.rgb / PI) + specularBRDF) * radiance * NdotL;
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
vec3 fresnelSchlick(
	const float cosTheta, 
	const vec3 surfaceReflection0Incidence,
	const vec3 surfaceReflection90Incidence
)
{
	return surfaceReflection0Incidence +
			(surfaceReflection90Incidence - surfaceReflection0Incidence) * 
				(pow(1.0 - clamp(cosTheta, 0.0, 1.0), 5.0));
}
