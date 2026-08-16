#version 460

layout(location = 0) in vec2 inUV;
layout(set = 0, binding = 2) uniform sampler2D preTonemapImage;

layout(location = 0) out vec4 toneMappedColor;

void main()
{

    toneMappedColor = texture(preTonemapImage, inUV);
}