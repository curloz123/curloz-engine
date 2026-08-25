/**
 * @file lights.hpp
 * @author curl0z
 * @brief Defines the GPU resources and interface used to provide lighting
 *        data to the renderer's shaders.
 *
 * This file contains the lighting shader data structures, descriptor
 * bindings, uniform/storage buffers, and the interface for initializing,
 * updating, and destroying the lighting resources.
 */
#pragma once

#include "renderer/lighting/lighting.hpp"
#include "renderer/lighting/lights.hpp"
#include "renderer/utility/ssbo.hpp"
#include "renderer/utility/ubo.hpp"

namespace clz::renderer
{
	/// @brief Light descriptor set point
	inline constexpr uint8_t LIGHT_SET_POINT = 2;

	/// @brief Light descriptor layout
	inline VkDescriptorSetLayout lightDescriptorLayout = VK_NULL_HANDLE;

	/**
	 * @brief Lighting information required by the shaders.
	 *
	 * Contains the number of point and spot lights currently active
	 * in the scene.
	 */
	struct ShaderLightData
	{
		uint32_t numPointLights;
		uint32_t numSpotLights;
	};

	/// @brief Light's descriptor UBO's memory
	inline UBOMemory lightDataUBOMemory{};
	/// @brief Light data UBO Bind point
	inline constexpr uint8_t LIGHT_DATA_BIND_POINT = 0;
	/// @brief Light Data shader UBO
	inline UBO lightDataUBO(sizeof(ShaderLightData), LIGHT_DATA_BIND_POINT);

	/// @brief Directional light UBO Memory
	inline UBOMemory dirUBOMemory{};
	/// @brief Directional light's bind point
	inline constexpr uint8_t DIR_LIGHT_BIND_POINT = 1;
	/// @brief Directional light's UBO
	inline UBO dirUBO(sizeof(DirectionalLight), DIR_LIGHT_BIND_POINT);

	/// @brief Point light's SSBO memory
	inline SSBOMemory pointSSBOMemory;
	/// @brief Point light's SSBO bind point
	inline constexpr uint8_t POINT_LIGHT_BIND_POINT = 2;
	/// @brief Actual Point light's SSBO
	inline SSBO pointSSBO(
		sizeof(PointLight) * MAX_POINT_LIGHTS,
		POINT_LIGHT_BIND_POINT
	);

} // namespace clz::renderer

namespace clz::renderer
{

	/**
	 * @brief Initializes the resources used to provide lighting data to shaders.
	 *
	 * Creates the lighting uniform/storage buffers and the descriptor set
	 * layout used to expose them to the fragment shader.
	 *
	 * @return true if all lighting resources were initialized successfully,
	 *         false otherwise.
	 */
	bool initLightDescriptor();

	/**
	 * @brief Updates lighting data for the current frame.
	 *
	 * Uploads the current light counts, directional-light data, and
	 * point-light data to their corresponding GPU buffers.
	 */
	void updateLightDescriptor();

	/**
	 * @brief Destroys the resources used by the lighting descriptor system.
	 *
	 * Releases the lighting descriptor set layout and all associated
	 * uniform and storage buffers.
	 */
	void destroyLightDescriptor();

} // namespace clz::renderer
