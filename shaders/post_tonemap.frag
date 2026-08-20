#version 460

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
		    uv.x *= PC.aspectRatio;
		    float dist = length(uv);

		    vec2 offset = uv * dist * PC.chromaticAberrationStrength;

		    float r = texture(tonemappedImage, inUV + offset).r;
		    float g = texture(tonemappedImage, inUV).g;
		    float b = texture(tonemappedImage, inUV - offset).b;

		    outColor.rgb = vec3(r, g, b);
	}

	if (enableVignette)
	{
		vec2 UV = inUV - vec2(0.5);
		UV.x *= PC.aspectRatio;
		float distUV = length(UV);
		float vignette	=  1.0 - smoothstep(PC.vignetteStart, PC.vignetteEnd, distUV);

		outColor *= vignette;
	}


}
