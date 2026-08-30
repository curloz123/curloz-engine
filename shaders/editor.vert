#version 460

// In's
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

// Uniform's

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


layout(set = 0, binding = 0) uniform CameraUBO
{
	mat4 projection;
	mat4 view;
	vec4 cameraPos;
} cameraUBO;

// Out's
layout(location = 0) out vec2 outUV;

void main()
{
   	gl_Position = cameraUBO.projection * cameraUBO.view * PC.model * vec4(inPosition, 1.0);
    	outUV = inUV;
}
