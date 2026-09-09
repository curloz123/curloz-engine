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
	float edgeFadeNear;
	float edgeFadeFar;
	
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
	    float dist = length(uv);
	    vec2 direction = (dist > 0.0001) ? normalize(uv) : vec2(0.0);
	    vec2 offset = direction * PC.chromaticAberrationStrength * dist * dist;

	    float edgeFade = 1.0 - smoothstep(PC.edgeFadeNear, PC.edgeFadeFar, dist);
	    offset *= edgeFade;

	    float r = texture(tonemappedImage, inUV - offset).r;
	    float g = texture(tonemappedImage, inUV).g;
	    float b = texture(tonemappedImage, inUV + offset).b;

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
