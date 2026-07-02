#include "renderer/shaderdata/descriptor/uniformbuffers.hpp"
#include "core/logs.hpp"
#include "core/time.hpp"
#include "math/localtransform.hpp"
#include "math/worldtransform.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/offsetalignment.hpp"
#include "renderer/vk_types.hpp"
#include <memory.h>
#include "renderer/camera/camera.hpp"

#include "math/localtransform.hpp"
#include <cstddef>
#include "math/angle.hpp"
#include "renderer/camera/cameramatrices.hpp"

namespace clz::renderer
{
	bool createUniformBuffers()
	{
		// First resize all the UBO's
		r_transformUBOOffsets.resize(r_FRAMES_IN_FLIGHT);
		r_transformUBOMapped.resize(r_FRAMES_IN_FLIGHT);

		// Second calculate UBO offsets and total UBO memory required
		r_totalUBOMemorySize = 0;
		for (auto i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			r_transformUBOOffsets[i] = r_totalUBOMemorySize;
			r_totalUBOMemorySize = getNextUBOOffsetAlignment(r_totalUBOMemorySize + sizeof(TransformUBO));
		}

		// Third create the total buffer
		if (!createBuffer(r_uniformBuffers, r_uniformBuffersMemory, "Main UBO",
				r_totalUBOMemorySize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			clz::log::error("Failed to create uniform buffers!");
			return false;
		}
		vkMapMemory(clz::renderer::r_deviceContext.device, r_uniformBuffersMemory, 0,
				r_totalUBOMemorySize, 0, &r_uniformBuffersMemoryMapped);

		for (auto i = 0; i<r_FRAMES_IN_FLIGHT; ++i)
		{
			r_transformUBOMapped[i] = static_cast<std::byte*>(r_uniformBuffersMemoryMapped) + r_transformUBOOffsets[i];
		}

		return true;
	}

	void updateUniformBuffers(const uint32_t currentFrame)
	{
		TransformUBO ubo;
		ubo.view = camera::getViewMatrix();
		ubo.projection  = camera::getProjectionMatrix();
		memcpy(r_transformUBOMapped[currentFrame], &ubo, sizeof(ubo));
	}

	void destroyUniformBuffers()
	{
		vkDestroyBuffer(clz::renderer::r_deviceContext.device, r_uniformBuffers, nullptr);
		vkFreeMemory(clz::renderer::r_deviceContext.device, r_uniformBuffersMemory, nullptr);
	}
}
