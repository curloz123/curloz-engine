#pragma once

#include "./camera.hpp"
#include "./lights.hpp"
#include "./post_process.hpp"
#include "./texture.hpp"
#include <vulkan/vulkan_core.h>

namespace clz::renderer
{
	inline CameraDescriptorInterface CameraDescriptor;
	inline LightDescriptorInterface LightDescriptor;
	inline PostProcessDescriptorInterface PostProcessDescriptor;
	inline TextureDescriptorInterface TextureDescriptor;

	inline VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	inline constexpr size_t NUM_DESCRIPTORS = 4;

	/// Must be initialized only after device context has been created
	bool initializePipelineData();
	void destroyPipelineData();

	inline VkDescriptorSetLayout EmptyLayout = VK_NULL_HANDLE;
	bool initializeEmptyDescriptorLayout();
	VkDescriptorSetLayout getEmptyLayout();
	void destroyEmptyLayout();
}
