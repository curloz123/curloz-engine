#version 460

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D renderTarget;
layout(push_constant) uniform PushConstants
{
    uint mode;

    float exposure;
} PC;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(renderTarget, inUV);
}