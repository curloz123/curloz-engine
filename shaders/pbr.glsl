/**
 * @file pbr.glsl
 * @author curl0z
 * @brief Provides all pbr related functions
 * like material structs etc...
 */

 #version 460

float distributionGGX(
	const vec3 normal,
	const vec3 halfway,
	const float roughness
)
{
	const float alphaSquare = pow(max(roughness, 0.001), 4);
	const float NdotH_Square = pow(max(dot(normal, halfway), 0.0), 2.0);
	const float denom = PI * pow((NdotH_Square * (alphaSquare - 1) + 1), 2);

	return alphaSquare / denom;
}
float geometrySchlickGGX(
	const float NdotV,
	const float roughness
)
{
	const float k = pow((roughness + 1), 2) / 8.0;
	return NdotFoo / ((NdotFoo * (1 - k)) + k);
}
float geometrySmith(
	const vec3 normal,
	const vec3 view,
	const vec3 lightPos,
	const float roughness
)
{
	const float NdotV = max(dot(normal, view), 0.0);
	const float NdotL = max(dot(normal, lightPos), 0.0);
	const float GGX1 = geometrySchlickGGX(NdotV, roughness);
	const float GGX2 = geometrySchlickGGX(NdotL, roughness);

	return GGX1 * GGX2;
}

vec3 fresnelSchlick(
	const float cosTheta,
	const vec3 surfaceReflection0Incidence,
	const vec3 surfaceReflection90Incidence
)
{
	return surfaceReflection0Incidence +
		surfaceReflection90Incidence - surfaceReflection0Incidence) *
		(pow(1.0 - clamp(cosTheta, 0.0, 1.0), 5.0));
}
