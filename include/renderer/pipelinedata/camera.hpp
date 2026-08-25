/**
 * @file camera.hpp
 * @author curl0z
 * @brief Camera descriptor resources and everything is stored here
 */
#pragma once

#include "math/mat4x4.hpp"
#include "math/vec4.hpp"
#include "renderer/utility/ubo.hpp"

namespace clz::renderer
{
	/// @brief Camera set point in main shader
	inline constexpr uint8_t CAMERA_SET_POINT = 0;
	/// @brief Camera bind point in main shader
	inline constexpr uint8_t CAMERA_BIND_POINT = 0;
	/// @brief Camera's UBO memory
	inline UBOMemory cameraUBOMemory{};
	/// @brief Camera's descriptor set layout
	inline VkDescriptorSetLayout cameraDescriptorLayout = VK_NULL_HANDLE;
	/// @struct CameraShaderUBO
	/// @brief Hold's shader's layout UBO, that will be passes every frame
	struct CameraShaderUBO
	{
		math::mat4 projection;
		math::mat4 view;
		math::vec4 cameraPos;
	};
	/// @brief Camera's UBO handle
	inline UBO cameraUBO(sizeof(CameraShaderUBO), CAMERA_BIND_POINT);

} // namespace clz::renderer

/// --- 3. UBO functions ---
namespace clz::renderer
{
	/// @brief Initializes camera descriptor resources
	/// @return True on succesful creation, false otherwise
	bool initCameraDescriptor();

	/// @brief Update's camera descriptor every frame
	/// @param cameraShaderUBO Shader's UBO data
	void updateCameraDescriptor(const CameraShaderUBO& cameraShaderUBO);

	/// @brief Destroy's camera descriptor resources
	void destroyCameraDescriptor();

} // namespace clz::renderer
