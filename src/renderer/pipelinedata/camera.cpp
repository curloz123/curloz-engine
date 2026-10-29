/**
 * @file ubo.cpp
 * @author curl0z
 * @brief Implementation of Pipeline input's Uniform buffers
 */

#include "renderer/pipelinedata/camera.hpp"
#include "core/logs.hpp"
#include "renderer/model/model.hpp"
#include "renderer/utility/descriptor.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	bool CameraDescriptorInterface::initialize()
	{
		UniformBufferCreateInfo uboCI
		{
			.size = sizeof(CameraShaderUBO),
			.bindPoint = CAMERA_BIND_POINT,
		};
		/// Creating UBO
		if (!createUniformBuffer(uboCI, m_ubo, "camera uniform buffer"))
		{
			clz::log::error("Could not create camera uniform buffers!");
			return false;
		}

		/// creating layout
		const std::vector<uint32_t> bindPoints = {
			CAMERA_BIND_POINT,
			MODEL_MATRIX_BIND_POINT
		};
		const std::vector<VkDescriptorType> descriptorTypes = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		};
		const std::vector<uint32_t> descriptorCounts = {1, 1};
		const std::vector<VkShaderStageFlags> shaderStages = {
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_VERTEX_BIT,
		};
		if (!createDescriptorLayout(
				m_descriptorLayout,
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

	void CameraDescriptorInterface::destroy()
	{
		destroyDescriptorSetLayout(m_descriptorLayout);
		destroyUniformBuffer(m_ubo);
	}

	std::vector<VkDescriptorPoolSize> CameraDescriptorInterface::getRequiredPoolSize()
	{
		std::vector<VkDescriptorPoolSize> cameraUBOPoolSizes(2);

		cameraUBOPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraUBOPoolSizes[0].descriptorCount = r_FRAMES_IN_FLIGHT;

		cameraUBOPoolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		cameraUBOPoolSizes[1].descriptorCount = r_FRAMES_IN_FLIGHT;

		return cameraUBOPoolSizes;
	}


	bool CameraDescriptorInterface::allocateDescriptorSets(VkDescriptorPool descriptorPool)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = r_FRAMES_IN_FLIGHT;
		const std::vector<VkDescriptorSetLayout> layouts(
			r_FRAMES_IN_FLIGHT,
			m_descriptorLayout
		);
		allocInfo.pSetLayouts = layouts.data();

		m_descriptorSets.resize(r_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(
				r_deviceContext.device,
		    		&allocInfo,
				m_descriptorSets.data()
		) != VK_SUCCESS)
		{
			clz::log::error("vulkan could not allocate camera descriptor sets");
			return false;
		}

		for (size_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			setHandleName(
				reinterpret_cast<uint64_t>(m_descriptorSets[i]),
				VK_OBJECT_TYPE_DESCRIPTOR_SET,
				("camera descriptor [frame " + std::to_string(i) + "]").c_str()
			);
		}

		return true;
	}

	void CameraDescriptorInterface::updateVPDescriptorSets()
	{
		for (size_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo cameraInfo = {};
			cameraInfo.buffer = m_ubo.buffer;
			cameraInfo.offset = m_ubo.offsets[i];
			cameraInfo.range = m_ubo.uboSize;

			VkWriteDescriptorSet cameraWrite{};
			cameraWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			cameraWrite.dstSet = m_descriptorSets[i];
			cameraWrite.dstBinding = CAMERA_BIND_POINT;
			cameraWrite.dstArrayElement = 0;
			cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			cameraWrite.descriptorCount = 1;
			cameraWrite.pBufferInfo = &cameraInfo;

			vkUpdateDescriptorSets(r_deviceContext.device, 1, &cameraWrite, 0, nullptr);
		}
	}
	void CameraDescriptorInterface::updateMDescriptorSets()
	{
		const auto numMatrices = ModelManager.getMatrixManager().getMatrixArrayLimit();
		if (numMatrices == 0)
			return;

		const auto& ssbo = ModelManager.getMatrixManager().getStorageBuffer();


		for (size_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo cameraInfo = {};
			cameraInfo.buffer = ssbo.buffer;
			cameraInfo.offset = ssbo.offsets[i];
			cameraInfo.range = ssbo.ssboSize;

			VkWriteDescriptorSet cameraWrite{};
			cameraWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			cameraWrite.dstSet = m_descriptorSets[i];
			cameraWrite.dstBinding = MODEL_MATRIX_BIND_POINT;
			cameraWrite.dstArrayElement = 0;
			cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			cameraWrite.descriptorCount = 1;
			cameraWrite.pBufferInfo = &cameraInfo;

			vkUpdateDescriptorSets(r_deviceContext.device, 1, &cameraWrite, 0, nullptr);
		}
	}
} // namespace clz::renderer
