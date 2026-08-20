#version 460

layout(location = 0) in vec2 inUV;
layout(binding = 0) uniform sampler2D renderTargetImage;
layout(binding = 1) uniform sampler2D bloomSampledImage;
layout(binding = 2) uniform sampler2D horizontalSampledImage;
layout(binding = 3) uniform sampler2D verticalSampledImage;

layout(push_constant) uniform PushConstants
{
	uint bloomBits;
	uint bloomStrength;
} PC;

layout(location = 0) out vec4 outColor;

vec3 horizontalBlur(sampler2D image)
{
	const float weights[5] = float[] (
			0.227027, 
			0.1945946, 
			0.1216216, 
			0.054054, 
			0.016216
	);
	const vec2 tex_offset = 1.0 / textureSize(image, 0);
	vec3 result = texture(image, inUV).rgb * weights[0];
	for (int i = 1; i < 5; ++i)
	{
		result += texture(image, inUV + vec2(tex_offset.x * i, 0.0)).rgb * 
				weights[i];
		result += texture(image, inUV - vec2(tex_offset.x * i, 0.0)).rgb * 
				weights[i];
	}

	return result;
}
vec3 verticalBlur(sampler2D image)
{
	const float weights[5] = float[] (
			0.227027, 
			0.1945946, 
			0.1216216, 
			0.054054, 
			0.016216
	);
	const vec2 tex_offset = 1.0 / textureSize(image, 0);
	vec3 result = texture(image, inUV).rgb * weights[0];
	for (int j = 1; j < 5; ++j)
	{
		result += texture(image, inUV + vec2(0.0, tex_offset.y * j)).rgb * 
				weights[j];
		result += texture(image, inUV - vec2(0.0, tex_offset.y * j)).rgb * 
				weights[j];
	}

	return result;
}

void main()
{
	const bool firstIteration 	= (PC.bloomBits & (1 << 0)) != 0u;
	const bool horizontal		= (PC.bloomBits & (1 << 1)) != 0u;
	const bool vertical 		= (PC.bloomBits & (1 << 2)) != 0u;

	if (firstIteration)
	{
		outColor = vec4(horizontalBlur(bloomSampledImage), 1.0);
		return;
	}
	else if (horizontal)
	{
		outColor = vec4(horizontalBlur(verticalSampledImage), 1.0);
		return;
	}
	else if (vertical)
	{
		outColor = vec4(verticalBlur(horizontalSampledImage), 1.0);
		return;
	}
	else
	{
		vec3 hdrColor = texture(verticalSampledImage, inUV).rgb;
		vec3 bloomColor = texture(renderTargetImage, inUV).rgb;
		hdrColor += bloomColor;
		outColor = vec4(hdrColor, 1.0);
		return;
	}

}
