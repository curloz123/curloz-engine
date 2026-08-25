#version 460

layout(location = 0) in vec2 inUV;
layout(binding = 3) uniform sampler2D bloomedImage; // horizontal bloom image
layout(push_constant) uniform PushConstants
{
	float exposure;
} PC;

layout(location = 0) out vec4 toneMappedColor;

const float A = 0.15;
const float B = 0.50;
const float C = 0.10;
const float D = 0.20;
const float E = 0.02;
const float F = 0.30;
const float W = 11.2;

vec3 Uncharted2Tonemap(vec3 hdrColor)
{

	vec3 numerator = hdrColor * ((A * hdrColor) + C*B) + D*E;
	vec3 denominator = hdrColor * ((A * hdrColor) + B) + D*F;

	return (numerator / denominator) - E/F;
}

void main()
{
	vec4 hdrColor = texture(bloomedImage, inUV);
	vec3 exposed = hdrColor.rgb * exp2(PC.exposure);
	vec3 mapped = Uncharted2Tonemap(exposed);
	vec3 whiteScale = 1.0f / Uncharted2Tonemap(vec3(W));

	toneMappedColor = vec4(mapped * whiteScale, hdrColor.w);
}
