#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace clz::renderer
{
	struct UBO
	{
		VkDeviceSize uboSize;
		uint32_t uboBindingPoint;
		std::vector<VkDeviceSize> offsets;
		std::vector<void*> mapped;

		explicit UBO(const VkDeviceSize uboSize, const uint32_t uboBindingPoint):
			uboSize(uboSize), uboBindingPoint(uboBindingPoint),
			offsets({}), mapped({})
		{
		}
	};
	struct UBOMemory
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		void* mappedMemory;
		VkDeviceSize memorySize;
	};

	bool createUniformBuffers(const std::vector<UBO*>& uniformBuffers, UBOMemory& uboMemory);
	void destroyUniformBuffer(const UBOMemory& uboMemory);
}