#version 460

// In's
layout(location = 0) in vec2 inUV;

// Uniform's
layout(binding = 1) uniform sampler2D textures[256];

layout(push_constant) uniform PushConstants
{
	layout(offset = 64) uint textureIndex;
} pushConstant;


// Out's
layout(location = 0) out vec4 outColor;

void main()
{
	outColor = texture(textures[pushConstant.textureIndex], inUV);
}
