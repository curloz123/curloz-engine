#version 460

// In's
layout(location = 0) in vec2 inUV;
layout(location = 1) flat in uint inTextureIndex;

// Uniform's
layout(binding = 0) uniform sampler2D textures[256];

// Out's
layout(location = 0) out vec4 outColor;

void main()
{
	outColor = texture(textures[inTextureIndex], inUV);
}
