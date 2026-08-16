#version 460

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 3) uniform sampler2D tonemappedImage;
layout(push_constant) uniform PushConstants
{
    uint postProcessBits;
} PC;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(tonemappedImage, inUV);
}
