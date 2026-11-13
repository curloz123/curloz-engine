#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout(push_constant) uniform PushConstants
{
	mat4 mvp;
	vec4 baseFactor;
      	uint textureIndex;
       	uint samplerIndex;
} pushConstant;

layout(location = 0) out vec2 outUV;

void main()
{
	gl_Position = pushConstant.mvp * vec4(inPos, 1.0);
	outUV = inUV;
}
