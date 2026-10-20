/**
 * @file ubo.cpp
 * @author curl0z
 * @brief Implementation of the per-frame uniform buffer helper (see ubo.hpp).
 */

#include "renderer/utility/ubo.hpp"
#include "core/logs.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/offsetalignment.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{

	/// @copydoc createUniformBuffer
	bool createUniformBuffer(
		const UniformBufferCreateInfo& uboCreateInfo,
		UBO& rUBO,
		const std::string& bufferName
	)
	{
		static const auto MinUBOAlignmentSize = [&]() {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(r_deviceContext.physicalDevice, &properties);
			return properties.limits.minUniformBufferOffsetAlignment;
		}();
		auto getNearestOffset = [alignment = MinUBOAlignmentSize](VkDeviceSize prvOffset) {
			return (alignment + prvOffset - 1) & ~(alignment - 1);
		};

		VkDeviceSize totalUBOMemory = 0;
		rUBO.offsets.resize(r_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; i++)
		{
			rUBO.offsets[i] = totalUBOMemory;
			totalUBOMemory = getNearestOffset(totalUBOMemory + uboCreateInfo.size);
		}

		const auto& device = clz::renderer::r_deviceContext.device;
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = totalUBOMemory;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(
			device,
			&bufferInfo,
			nullptr,
			&rUBO.buffer
		) != VK_SUCCESS)
		{
			clz::log::error(
				"vulkan failed to create uniform buffer: " + bufferName,
				true
			);
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, rUBO.buffer, &memRequirements);
		UBOMemoryAllocInfo memAllocInfo{
			.size = memRequirements.size,
			.alignment = memRequirements.alignment
		};
		if (!carveUBOMemory(memAllocInfo, rUBO.memory))
		{
			clz::log::error(
				"could not carve memory for ubo: " + bufferName,
				true
			);
		}

		setHandleName(
			reinterpret_cast<uint64_t>(rUBO.buffer),
			VK_OBJECT_TYPE_BUFFER,
			bufferName.c_str()
		);

		std::byte* mapped = getUBOMappedPointer(rUBO.memory);

		rUBO.mapped.resize(r_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			rUBO.mapped[i] = reinterpret_cast<void*>(mapped + rUBO.offsets[i]);
		}

		rUBO.uboSize = uboCreateInfo.size;

		return true;
	}

	/// @copydoc destroyUniformBuffer
	void destroyUniformBuffer(UBO& rUBO, const bool explicitDestroy)
	{
		rUBO.mapped.clear();
		vkDestroyBuffer(r_deviceContext.device, rUBO.buffer, nullptr);
		if (explicitDestroy)
			freeUBOMemory(rUBO.memory);
	}

} // namespace clz::renderer
