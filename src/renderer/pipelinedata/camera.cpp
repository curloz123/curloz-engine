/**
 * @file ubo.cpp
 * @author curl0z
 * @brief Implementation of Pipeline input's Uniform buffers
 */

#include "renderer/pipelinedata/camera.hpp"
#include "core/logs.hpp"
#include "memory.h"
#include "renderer/lighting/lighting.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/descriptor.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/offsetalignment.hpp"

namespace clz::renderer
{
	bool initCameraDescriptor()
	{
		/// Creating UBO
		if (!createUniformBuffer(cameraUBO, cameraUBOMemory, "camera uniform buffer"))
		{
			clz::log::error("Could not create camera uniform buffers!");
		}

		/// creating layout
		const std::vector<uint32_t> bindPoints = {CAMERA_BIND_POINT};
		const std::vector<VkDescriptorType> descriptorTypes = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		};
		const std::vector<uint32_t> descriptorCounts = {1};
		const std::vector<VkShaderStageFlags> shaderStages = {VK_SHADER_STAGE_VERTEX_BIT};
		if (!createDescriptorLayout(
			    cameraDescriptorLayout,
			    bindPoints,
			    descriptorTypes,
			    descriptorCounts,
			    shaderStages,
			    "camera ubo layout"
		    ))
		{
			clz::log::error("Could not create camera ubo layout!");
			return false;
		}

		return true;
	}

	void updateCameraDescriptor(const CameraShaderUBO& cameraShaderUBO)
	{
		memcpy(cameraUBO.mapped[r_currentFrame], &cameraShaderUBO, sizeof(CameraShaderUBO));
	}

	void destroyCameraDescriptor()
	{
		destroyDescriptorSetLayout(cameraDescriptorLayout);
		destroyUniformBuffer(cameraUBO, cameraUBOMemory);
	}

} // namespace clz::renderer