#pragma once

#include "math/vec4.hpp"

namespace clz::renderer
{

enum class LightType
{
	DIRECTIONAL,
	POINT,
	SPOT
};

struct DirectionalLight
{
	/// @brief direction of directional light
	/// x,y,z decides direction
	/// w is just for padding
	math::vec4 direction = math::vec4(0.0f, -1.0f, 0.0f, 0.0f);

	/// @brief color of directional light
	/// x,y,z decides direction
	/// w decides intensity
	math::vec4 color = math::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	DirectionalLight() = default;
};

struct PointLight
{
	/// @brief positon of point light
	/// x,y,z decides position
	/// w decides range
	math::vec4 position = math::vec4(0.0f, 0.0f, 0.0f, 10.0f);

	/// @brief color of point light
	/// x,y,z decides color
	/// w decides intensity
	math::vec4 color = math::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	/**
	 * @brief Attenuation values
	 * Below is the table for attenuation values

		Distance	Constant 	Linear 	Quadratic
		7 		1.0 		0.7 	1.8
		13 		1.0 		0.35 	0.44
		20 		1.0 		0.22 	0.20
		32 		1.0 		0.14 	0.07
		50 		1.0 		0.09 	0.032
		65 		1.0 		0.07 	0.017
		100 		1.0 		0.045 	0.0075
		160 		1.0 		0.027 	0.0028
		200 		1.0 		0.022 	0.0019
		325 		1.0 		0.014 	0.0007
		600 		1.0 		0.007 	0.0002
		3250 		1.0 		0.0014 	0.000007

	 * x component decides constant (always 1.0f)
	 * y component decides linear value
	 * z component decides quadratic value
	 * w is just padding
	 *
	 * Formula for attenuation is:
	 * 1.0f / (Kc + Kl*d + Kq*(d^2))
	 * @note Constant is always kept 1.0
	 */
	math::vec4 attenuation = math::vec4(1.0f, 0.09f, 0.032f, 1.0f);

	PointLight() = default;
};

struct SpotLight
{
	/// @brief direction of spotlight
	/// x,y,z decides direction
	/// w is just for padding
	math::vec4 direction = math::vec4(0.0f, 0.0f, -1.0f, 0.0f);

	/// @brief position of spotlight
	/// x,y,z decides position
	/// w decides range
	math::vec4 position = math::vec4(0.0f, 0.0f, 0.0f, 10.0f);

	/// @brief color of point light
	/// x,y,z decides color
	/// w decides intensity
	math::vec4 color = math::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	/**
	 * @brief Attenuation values
	 * Below is the table for attenuation values

		Distance	Constant 	Linear 	Quadratic
		7 		1.0 		0.7 	1.8
		13 		1.0 		0.35 	0.44
		20 		1.0 		0.22 	0.20
		32 		1.0 		0.14 	0.07
		50 		1.0 		0.09 	0.032
		65 		1.0 		0.07 	0.017
		100 		1.0 		0.045 	0.0075
		160 		1.0 		0.027 	0.0028
		200 		1.0 		0.022 	0.0019
		325 		1.0 		0.014 	0.0007
		600 		1.0 		0.007 	0.0002
		3250 		1.0 		0.0014 	0.000007

	 * x component decides constant (always 1.0f)
	 * y component decides linear value
	 * z component decides quadratic value
	 * w is just padding
	 *
	 * Formula for attenuation is:
	 * 1.0f / (Kc + Kl*d + Kq*(d^2))
	 * @note Constant is always kept 1.0
	 */
	math::vec4 attenuation = math::vec4(1.0f, 0.09f, 0.032f, 1.0f);

	/// @brief spotlight cutoff values
	/// x decides inner cutoff angle
	/// y decides outer cutoff angle
	math::vec4 cutoff = math::vec4(25.0f, 35.0f, 0.0f, 1.0f);

	SpotLight() = default;
};

}