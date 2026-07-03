#version 460

// In's
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

// Uniform's

layout(push_constant) uniform PushConstants
{
	mat4 model;
} pushConstant;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 projection;
	mat4 view;
} ubo;

// Out's
layout(location = 0) out vec2 outUV;



void main()
{
   	gl_Position = ubo.projection * ubo.view * pushConstant.model * vec4(inPosition, 1.0);
	outUV = inUV;
}
