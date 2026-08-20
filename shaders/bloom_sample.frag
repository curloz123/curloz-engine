#version 460

layout(location = 0) in vec2 inUV;

layout(binding = 0) uniform sampler2D renderTarget;
/* layout(push_constant) uniform PushConstants */
/* { */
/*    	uint mode; */
/*     	float exposure; */
/* } PC; */

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 fragColor = texture(renderTarget, inUV);
	/* outColor = vec4(vec3(0.0), fragColor.w); */
	/* if (fragColor.r > 1.0) */
	/* 	outColor.r = fragColor.r; */
	/* if (fragColor.g > 1.0) */
	/* 	outColor.g = fragColor.g; */
	/* if (fragColor.b > 1.0) */
	/* 	outColor.b = fragColor.b; */
		
	float brightness = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0)
		outColor = fragColor;
	else
		outColor = vec4(vec3(0.0), 1.0);
}
