#version 460

#define SAMPLES 10.0
#define OFFSET vec2(0.01, 0.01)
#define CONTRAST 2.0

layout(location = 0) in vec2 inUV;
layout(binding = 4) uniform sampler2D tonemappedImage;

layout(push_constant) uniform PushConstants
{
   	uint postProcessBits;
	
	float aspectRatio;

	float vignetteStart;
	float vignetteEnd;

	float chromaticAberrationStrength;
	
} PC;

layout(location = 0) out vec4 outColor;

void main()
{
	bool enableVignette 		= (PC.postProcessBits & 1u) != 0u;
	bool enableChromaticAberration 	= (PC.postProcessBits & 2u) != 0u;

    	outColor = texture(tonemappedImage, inUV);

	if (enableChromaticAberration)
	{
		vec2 uv = inUV - vec2(0.5);
		vec2 aspectUV = uv;
		aspectUV.x *= PC.aspectRatio;
		float dist = dot(aspectUV, aspectUV);

		vec2 offset = uv * (PC.chromaticAberrationStrength * dist);

		vec2 rUV = clamp(inUV + offset, 0.0, 1.0);
		vec2 bUV = clamp(inUV - offset, 0.0, 1.0);
		float r = texture(tonemappedImage,  rUV).r;
		float g = texture(tonemappedImage, inUV).g;
		float b = texture(tonemappedImage,  bUV).b;

		outColor.rgb = vec3(r, g, b);
	}

	if (enableVignette)
	{
		vec2 UV = inUV - vec2(0.5);
		UV.x *= PC.aspectRatio;
		float dist = length(UV);
		float maxDist = length(vec2(0.5 * PC.aspectRatio, 0.5));
		float radius = dist / maxDist;
		float vignette	=  1.0 - smoothstep(PC.vignetteStart, PC.vignetteEnd, radius);

		outColor *= vignette;
	}


}
