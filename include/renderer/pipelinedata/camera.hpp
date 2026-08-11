
#pragma once

#include "math/mat4x4.hpp"
#include "renderer/lighting/lights.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/ubo.hpp"

namespace clz::renderer
{

	inline constexpr uint8_t CAMERA_SET_POINT = 0;
	inline constexpr uint8_t CAMERA_BIND_POINT = 0;
	inline UBOMemory cameraUBOMemory{};
	inline VkDescriptorSetLayout cameraDescriptorLayout = VK_NULL_HANDLE;
	struct CameraShaderUBO
	{
		math::mat4 projection;
		math::mat4 view;
		math::vec4 cameraPos;
	};
	inline UBO cameraUBO(sizeof(CameraShaderUBO), CAMERA_BIND_POINT);

} // namespace clz::renderer

/// --- 3. UBO functions ---
namespace clz::renderer
{

	bool initCameraDescriptor();
	void updateCameraDescriptor(const CameraShaderUBO& cameraShaderUBO);
	void destroyCameraDescriptor();

} // namespace clz::renderer