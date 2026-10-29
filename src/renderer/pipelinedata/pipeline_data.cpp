#include "renderer/pipelinedata/pipeline_data.hpp"
#include "core/logs.hpp"
#include "renderer/pipelinedata/camera.hpp"
#include "renderer/pipelinedata/lights.hpp"
#include "renderer/pipelinedata/post_process.hpp"
#include "renderer/pipelinedata/texture.hpp"
#include "renderer/vk_types.hpp"
#include <iterator>
#include <vulkan/vulkan_core.h>
#include "renderer/utility/descriptor.hpp"

namespace clz::renderer
{
	bool initializePipelineData()
	{
		if (!CameraDescriptor.initialize())
		{
			clz::log::error("Could not initialize camera descriptor", true);
			return false;
		}
		if (!LightDescriptor.initialize())
		{
			clz::log::error("Could not initialize light descriptor", true);
			return false;
		}
		if (!PostProcessDescriptor.initialize())
		{
			clz::log::error("Could not initialize post-process descriptor", true);
			return false;
		}
		if (!TextureDescriptor.initialize())
		{
			clz::log::error("Could not initialize texture descriptor", true);
			return false;
		}
		if (!initializeEmptyDescriptorLayout())
		{
			clz::log::error("Could not initialize empty descriptor layout", true);
			return false;
		}

		auto cameraPoolSize = CameraDescriptor.getRequiredPoolSize();
		auto lightPoolSize = LightDescriptor.getRequiredPoolSizes();
		auto postProcessPoolSize = PostProcessDescriptor.getRequiredPoolSizes();
		auto texturePoolSize = TextureDescriptor.getRequiredPoolSizes();

		std::vector<VkDescriptorPoolSize> totalPoolSize;
		totalPoolSize.reserve(
			cameraPoolSize.size() +
			lightPoolSize.size() +
			postProcessPoolSize.size() +
			texturePoolSize.size()
		);
		totalPoolSize.insert(
			totalPoolSize.end(),
			std::make_move_iterator(cameraPoolSize.begin()),
			std::make_move_iterator(cameraPoolSize.end())
		);
		totalPoolSize.insert(
			totalPoolSize.end(),
			std::make_move_iterator(lightPoolSize.begin()),
			std::make_move_iterator(lightPoolSize.end())
		);
		totalPoolSize.insert(
			totalPoolSize.end(),
			std::make_move_iterator(postProcessPoolSize.begin()),
			std::make_move_iterator(postProcessPoolSize.end())
		);
		totalPoolSize.insert(
			totalPoolSize.end(),
			std::make_move_iterator(texturePoolSize.begin()),
			std::make_move_iterator(texturePoolSize.end())
		);

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolInfo.poolSizeCount = static_cast<uint32_t>(totalPoolSize.size());
		poolInfo.pPoolSizes = totalPoolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(r_FRAMES_IN_FLIGHT) * 4;

		if (vkCreateDescriptorPool(
			    r_deviceContext.device,
			    &poolInfo,
			    nullptr,
			    &DescriptorPool
		) != VK_SUCCESS)
		{
			clz::log::error("Could not create descriptor pool");
			return false;
		}

		CameraDescriptor.allocateDescriptorSets(DescriptorPool);
		LightDescriptor.allocateDescriptorSets(DescriptorPool);
		PostProcessDescriptor.allocateDescriptorSets(DescriptorPool);
		TextureDescriptor.allocateDescriptorSets(DescriptorPool);

		/// update only camera and light descriptors
		CameraDescriptor.updateVPDescriptorSets();
		LightDescriptor.updateDescriptorSets();

		return true;
	}

	void destroyPipelineData()
	{
		destroyEmptyLayout();
		TextureDescriptor.destroy();
		PostProcessDescriptor.destroy();
		LightDescriptor.destroy();
		CameraDescriptor.destroy();

		vkDestroyDescriptorPool(
			r_deviceContext.device,
			DescriptorPool,
			nullptr
		);
	}


	bool initializeEmptyDescriptorLayout()
	{
		constexpr std::array<uint32_t, 0> bindPoints;
		constexpr std::array<VkDescriptorType, 0> descriptorTypes;
		constexpr std::vector<uint32_t> descriptorCounts;
		const std::vector<VkShaderStageFlags> shaderStages;
		if (!createDescriptorLayout(
				EmptyLayout,
			    	bindPoints,
				descriptorTypes,
			    	descriptorCounts,
				shaderStages,
				"camera ubo layout",
				true
		))
		{
			clz::log::error("Could not create camera ubo layout!");
			return false;
		}

		return true;
	}
	void destroyEmptyLayout()
	{
		destroyDescriptorSetLayout(EmptyLayout);
	}
	VkDescriptorSetLayout getEmptyLayout()
	{
		return EmptyLayout;
	}
} // namespace clz::renderer
