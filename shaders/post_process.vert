#version 460

vec3 positions[] = vec3[](
        vec3(1.0, -1.0, 0.0),
        vec3(-1.0, -1.0, 0.0),
        vec3(-1.0, 1.0, 0.0),
        vec3(-1.0, 1.0, 0.0),
        vec3(1.0, 1.0, 0.0),
        vec3(1.0, -1.0, 0.0)
);
vec2 UV[] = vec2[](
        vec2(1.0, 0.0),
        vec2(0.0, 0.0),
        vec2(0.0, 1.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0),
        vec2(1.0, 0.0)
);

layout(location = 0) out vec2 outUV;

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 1.0);
    outUV = UV[gl_VertexIndex];
}