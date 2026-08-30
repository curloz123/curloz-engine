#version 460

// In's
layout(location = 0) in vec2 inUV;
layout(location = 1) flat in uint inTextureIndex;

// Uniform's
#define NULL_TEXTURE uint(-1)
layout(set = 1, binding = 0) uniform sampler2D textures[256];

layout(push_constant) uniform PushConstants
{
	mat4 model;

	vec4 baseColorFactor;
	uint baseTextureIndex;
	float metallicFactor;
	float roughnessFactor;
	uint metallic_roughnessTextureIndex;
	vec3 emissiveFactor;
	uint emissiveTextureIndex;
	float emissiveStrength;
	uint normalTextureIndex;
} PC;


// Out's
layout(location = 0) out vec4 outColor;

void main()
{
	vec4 base;
	if (PC.baseTextureIndex == NULL_TEXTURE)
	{
		base = PC.baseColorFactor;
	}
	else
	{
		base = texture(textures[PC.baseTextureIndex], inUV) * 
			PC.baseColorFactor;
	}

	vec3 emissiveColor = vec3(0.0);
	if (PC.emissiveTextureIndex != NULL_TEXTURE)
	{
		emissiveColor = texture(textures[PC.emissiveTextureIndex], inUV).rgb * 
				PC.emissiveFactor * PC.emissiveStrength;	
	}
	else
	{
		emissiveColor = PC.emissiveFactor * PC.emissiveStrength;
	}

	outColor = vec4(base.rgb + emissiveColor, 1.0);
}
