#version 460

layout(location = 0) in vec2 inUV;
layout(binding = 0) uniform sampler2D renderTargetImage;
layout(binding = 1) uniform sampler2D bloomSampledImage;

#define NUM_BLOOM_MIPS    	5
#define RENDER_TARGET_INDEX    -1
#define BLOOMED_IMAGE_INDEX    -1
layout(binding = 2) uniform sampler2D bloomMips[NUM_BLOOM_MIPS];

layout(push_constant) uniform PushConstants
{
	uint  bloomBits;
	int   downIndex;
	int   upIndex;
	float filterRadius;
	float bloomStrength;
} PC;

layout(location = 0) out vec4 outColor;

float RGBToLuminance(vec3 color)
{
	return dot(color, vec3(0.2126f, 0.7152f, 0.0722f));
}
float KarisAverage(vec3 color)
{
	const float T = 1.0;
	const float K = 0.2;
	const float B = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	float S = B - T + K;
	S = clamp(S, 0.0, K*2);
	S = (S*S)/(4*K);
	const float C = max(S, B-T);
	const float F = C / max(B, 1e-5);

	color *= F;
	float luma = RGBToLuminance(color);
	return 1.0 / (1.0 + luma);
}
vec3 getDownsampledColor(sampler2D srcImage, bool firstSample)
{
	// Take 13 samples around current texel:
	// a - b - c
	// - j - k -
	// d - e - f
	// - l - m -
	// g - h - i
	// === ('e' is the current texel) ===

	const vec2 texelSize = 1.0 / textureSize(srcImage, 0);
	const float x = texelSize.x;
	const float y = texelSize.y;

	const vec3 a = texture(srcImage, vec2(inUV.x - 2*x, inUV.y - 2*y)).rgb;
	const vec3 b = texture(srcImage, vec2(inUV.x      , inUV.y - 2*y)).rgb;
	const vec3 c = texture(srcImage, vec2(inUV.x + 2*x, inUV.y - 2*y)).rgb;

	const vec3 d = texture(srcImage, vec2(inUV.x - 2*x, inUV.y      )).rgb;
	const vec3 e = texture(srcImage, vec2(inUV.x      , inUV.y      )).rgb;
	const vec3 f = texture(srcImage, vec2(inUV.x + 2*x, inUV.y      )).rgb;

	const vec3 g = texture(srcImage, vec2(inUV.x - 2*x, inUV.y + 2*y)).rgb;
	const vec3 h = texture(srcImage, vec2(inUV.x      , inUV.y + 2*y)).rgb;
	const vec3 i = texture(srcImage, vec2(inUV.x + 2*x, inUV.y + 2*y)).rgb;

	const vec3 j = texture(srcImage, vec2(inUV.x -   x, inUV.y -   y)).rgb;
	const vec3 k = texture(srcImage, vec2(inUV.x +   x, inUV.y -   y)).rgb;

	const vec3 l = texture(srcImage, vec2(inUV.x -   x, inUV.y +   y)).rgb;
	const vec3 m = texture(srcImage, vec2(inUV.x +   x, inUV.y +   y)).rgb;

	if (!firstSample)
	{
		vec3 color = e * 0.125;
		color += (a+c+g+i) * 0.03125;
		color += (b+d+f+h) * 0.0625;
		color += (j+k+l+m) * 0.125;

		return color;
	}

	vec3 groups[5];
	groups[0] = (a+b+d+e) * (0.125f/4.0f);
	groups[1] = (b+c+e+f) * (0.125f/4.0f);
	groups[2] = (d+e+g+h) * (0.125f/4.0f);
	groups[3] = (e+f+h+i) * (0.125f/4.0f);
	groups[4] = (j+k+l+m) * (0.5f/4.0f);
	groups[0] *= KarisAverage(groups[0]);
	groups[1] *= KarisAverage(groups[1]);
	groups[2] *= KarisAverage(groups[2]);
	groups[3] *= KarisAverage(groups[3]);
	groups[4] *= KarisAverage(groups[4]);
	return groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
}
vec3 getUpsampledColor(sampler2D srcImage)
{
        // Take 9 samples around current texel:
	// a - b - c
	// d - e - f
	// g - h - i
	// === ('e' is the current texel) ===

	const vec2 texelSize = 1.0 / textureSize(srcImage, 0);
	const float x = PC.filterRadius;
	const float y = PC.filterRadius;

	const vec3 a = texture(srcImage, vec2(inUV.x - x, inUV.y - y)).rgb;
	const vec3 b = texture(srcImage, vec2(inUV.x    , inUV.y - y)).rgb;
	const vec3 c = texture(srcImage, vec2(inUV.x + x, inUV.y - y)).rgb;

	const vec3 d = texture(srcImage, vec2(inUV.x - x, inUV.y      )).rgb;
	const vec3 e = texture(srcImage, vec2(inUV.x    , inUV.y      )).rgb;
	const vec3 f = texture(srcImage, vec2(inUV.x + x, inUV.y      )).rgb;

	const vec3 g = texture(srcImage, vec2(inUV.x - x, inUV.y + y)).rgb;
	const vec3 h = texture(srcImage, vec2(inUV.x    , inUV.y + y)).rgb;
	const vec3 i = texture(srcImage, vec2(inUV.x + x, inUV.y + y)).rgb;

	vec3 color = e * 4.0;
	color += (b+d+f+h) * 2.0;
	color += (a+c+g+i);
	color *= 1.0 / 16.0;

	return color;
}

void main()
{
	const bool downSample = (PC.bloomBits & (1 << 0)) != 0u;
	const bool upSample   = (PC.bloomBits & (1 << 1)) != 0u;
	const bool disabled   = (PC.bloomBits & (1 << 2)) != 0u;

	if (downSample)
	{
		if (PC.downIndex != RENDER_TARGET_INDEX)
		{
			outColor = vec4(getDownsampledColor(bloomMips[PC.downIndex - 1], false), 1.0);
			return;
		}

		outColor = vec4(getDownsampledColor(renderTargetImage, true), 1.0);
		return;

	}
	else if(upSample)
	{
		if (PC.upIndex != BLOOMED_IMAGE_INDEX)
		{
			outColor = vec4(getUpsampledColor(bloomMips[PC.upIndex + 1]), 1.0);
			return;
		}

		vec3 bloomColor = getUpsampledColor(bloomMips[0]);
		vec3 hdrColor   = texture(renderTargetImage, inUV).rgb;
		outColor = vec4(mix(hdrColor, bloomColor, PC.bloomStrength), 1.0);
		return;
	}
	else
	{
		outColor = texture(renderTargetImage, inUV);
		return;
	}

}
