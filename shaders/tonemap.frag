#version 460

layout(location = 0) in vec2 inUV;
layout(set = 0, binding = 2) uniform sampler2D preTonemapImage;
layout(push_constant) uniform PushConstants
{
	float exposure;
} PC;

layout(location = 0) out vec4 toneMappedColor;

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

vec3 Uncharted2Tonemap(vec3 hdrColor)
{

	vec3 numerator = hdrColor * ((A * hdrColor) + C*B) + D*E;
	vec3 denominator = hdrColor * ((A * hdrColor) + B) + D*F;

	return (numerator / denominator) - E/F;
}

void main()
{
	vec4 hdrColor = texture(preTonemapImage, inUV);
	vec3 mapped = Uncharted2Tonemap(hdrColor.rgb) * exp2(PC.exposure);
	vec3 whiteScale = 1.0f / Uncharted2Tonemap(vec3(W));

	toneMappedColor = vec4(mapped * whiteScale, hdrColor.w);
}
