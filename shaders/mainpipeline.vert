/**
 * @file mainpipeline.vert
 * @author curl0z
 * @brief main pipeline's vertex shader
 */

#version 460

// In's
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

// Uniform's

layout(push_constant) uniform PushConstants
{
	mat4 model;

	vec4 baseColorFactor;
	uint baseTextureIndex;
	float metallicFactor;
	float roughnessFactor;
	uint metallic_roughnessTextureIndex;
	uint normalTextureIndex;
} pushConstant;

layout(set = 0, binding = 0) uniform CameraUBO
{
	mat4 projection;
	mat4 view;
	vec4 cameraPos;
} u_Camera;

// Out's
layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outFragWPos;
layout(location = 3) out vec3 outCameraPos;
layout(location = 4) out vec3 outTangent;
layout(location = 5) out vec3 outBitangent;

void main()
{
   	gl_Position = u_Camera.projection * u_Camera.view * pushConstant.model * vec4(inPosition, 1.0);

	outUV = inUV;
	outNormal = normalize(mat3(transpose(inverse(pushConstant.model))) * inNormal);
	outFragWPos = vec3(pushConstant.model * vec4(inPosition, 1.0));
	outCameraPos = u_Camera.cameraPos.xyz;
	outTangent = inTangent.xyz;
	outBitangent = cross(inNormal, inTangent.xyz) * inTangent.w;
}
