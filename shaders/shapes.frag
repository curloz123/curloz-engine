#version 460

#define NULL_TEXTURE  0xFFFFFFFFu

layout (location = 0) in vec2 inUV;
layout(push_constant) uniform PushConstants
{
	mat4 mvp;
	vec4 baseFactor;
      	uint textureIndex;
       	uint samplerIndex;
} pushConstant;

layout(set = 1, binding = 0) uniform texture2D textures[1024];
layout(set = 1, binding = 1) uniform sampler samplers[1024];

layout (location = 0) out vec4 outColor;

void main()
{
	if (pushConstant.textureIndex == NULL_TEXTURE)
	{
		outColor = vec4(pushConstant.baseFactor.rgb, 1.0);
		return;
	}

	vec4 base = texture(
			sampler2D(
				textures[pushConstant.textureIndex],
				samplers[pushConstant.samplerIndex]
			),
			inUV
		) * pushConstant.baseFactor;
	outColor = vec4(base.rgb, 1.0);
}
