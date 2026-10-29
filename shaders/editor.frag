#version 460

// In's
layout(location = 0) in vec2 inUV;

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

	vec2 _pad;
};

// Uniform's
#define NULL_TEXTURE uint(-1)
layout(set = 1, binding = 0) uniform texture2D textures[1024];
layout(set = 1, binding = 1) uniform sampler samplers[1024];
layout(std430, set = 1, binding = 2) readonly buffer Materials
{
	Material materials[];
} s_Materials;

layout(push_constant) uniform PushConstants
{
	mat4 model;
	uint materialIndex;
} PC;


// Out's
layout(location = 0) out vec4 outColor;

void main()
{
	vec4 base;
	Material material = s_Materials.materials[PC.materialIndex];

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

	vec3 emissiveColor = vec3(0.0);
	if (material.emissiveTextureId != NULL_TEXTURE)
	{

		emissiveColor = texture(
					sampler2D(
						textures[material.emissiveTextureId],
						samplers[material.emissiveSamplerId]
					),
					inUV
				).rgb * material.emissiveFactor * material.emissiveStrength;
	}
	else
	{
		emissiveColor = material.emissiveFactor * material.emissiveStrength;
	}

	outColor = vec4(base.rgb + emissiveColor, 1.0);
}
