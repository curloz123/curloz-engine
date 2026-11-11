/**
 * @file body_editor.vert
 * @author curl0z
 * @brief Vertex shader of body editor pipeline
 */

#version 460


// In's
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;


// Push constant
layout(push_constant) uniform PushConstant
{
	mat4 mvp;
	uint materialIndex;
} pushConstant;

/// Out's
layout (location = 0) out vec2 outUV;

void main()
{
	gl_Position = pushConstant.mvp * vec4(inPos, 1.0);
	outUV = inUV;
}
