#version 460

layout(location = 0) in vec2 inUV;

layout(binding = 0) uniform sampler2D renderTarget;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 fragColor = texture(renderTarget, inUV);
		
	const float T = 1.0;
	const float K = 0.2;
	const float B = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	float S = B - T + K;
	S = clamp(S, 0.0, K*2);
	S = (S*S)/(4*K);
	const float C = max(S, B-T);
	const float F = C / max(B, 1e-5);

	outColor = vec4(fragColor.rgb * F, 1.0);
}
