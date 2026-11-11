/**
 * @file body_editor.frag
 * @author curl0z
 * @brief Fragment shader of body editor pipeline
 */

 #version 460

#define NULL_TEXTURE  0xFFFFu
#define NULL_MATERIAL 0xFFFFu

layout (location = 0) in vec2 inUV;

/// General material storage buffer.
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

// Push constant
layout(push_constant) uniform PushConstant
{
	mat4 mvp;
	uint materialIndex;
} pushConstant;

layout(set = 1, binding = 0) uniform texture2D textures[1024];

layout(set = 1, binding = 1) uniform sampler samplers[1024];

layout(std430, set = 1, binding = 2) readonly buffer Materials
{
	Material materials[];
} s_Materials;

layout (location = 0) out vec4 outColor;

void main()
{
	if (pushConstant.materialIndex == NULL_MATERIAL)
	{
		outColor = vec4(vec3(1.0), 1.0);
		return;
	}

	Material material = s_Materials.materials[pushConstant.materialIndex];

	vec4 base = material.baseFactor;
	if (material.baseTextureId != NULL_TEXTURE)
	{
		base *= texture(
				sampler2D(
					textures[material.baseTextureId],
					samplers[material.baseSamplerId]
				),
				inUV
			);
	}

	/// return base only as of now
	outColor = vec4(vec3(base), 1.0);
}
