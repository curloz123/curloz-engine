/**
 * @file lights.cpp
 * @author curl0z
 * @brief Implementation of the light descriptor interface.
 */
#include "renderer/pipelinedata/lights.hpp"
#include "renderer/lighting/lights.hpp"
#include "renderer/lighting/lighting.hpp"
#include "core/logs.hpp"
#include "renderer/utility/descriptor.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include <array>
#include <string>
#include <memory.h>
#include "entity/corecomponents.hpp"
#include "entity/componentmanager.hpp"
#include "renderer/rendercomponent.hpp"

namespace clz::renderer
{
	bool LightDescriptorInterface::initialize()
	{
		UniformBufferCreateInfo lightDataCI{
			.size = sizeof(ShaderLightData),
			.bindPoint = LIGHT_DATA_BIND_POINT
		};
		if (!createUniformBuffer(lightDataCI, m_lightDataUBO, "light data ubo"))
		{
			clz::log::error("could not create light data ubo buffer");
			return false;
		}

		UniformBufferCreateInfo dirLightCI{
			.size = sizeof(DirectionalLight) * MAX_DIR_LIGHTS,
			.bindPoint = DIR_LIGHT_BIND_POINT
		};
		if (!createUniformBuffer(dirLightCI, m_dirUBO, "directional light ubo"))
		{
			clz::log::error("could not create directional light uniform buffer");
			return false;
		}

		StorageBufferCreateInfo pointLightCI{
			.size = sizeof(PointLight) * MAX_POINT_LIGHTS,
			.bindPoint = POINT_LIGHT_BIND_POINT
		};
		if (!createStorageBuffer(pointLightCI, m_pointSSBO, "point light ssbo"))
		{
			clz::log::error("could not create point light ssbo buffer");
			return false;
		}

		constexpr std::array<uint32_t, 3> bindPoints = {
			LIGHT_DATA_BIND_POINT,
			DIR_LIGHT_BIND_POINT,
			POINT_LIGHT_BIND_POINT
		};
		constexpr std::array<VkDescriptorType, 3> descriptorTypes = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		};
		constexpr std::array<uint32_t, 3> descriptorCounts = {1, 1, 1};
		constexpr std::array<VkShaderStageFlags, 3> shaderStages = {
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT
		};
		if (!createDescriptorLayout(
			    m_descriptorLayout,
			    bindPoints,
			    descriptorTypes,
			    descriptorCounts,
			    shaderStages,
			    "light descriptor layout"
		    ))
		{
			clz::log::error("Could not create light descriptor layout!");
			return false;
		}

		return true;
	}

	void LightDescriptorInterface::destroy()
	{
		destroyDescriptorSetLayout(m_descriptorLayout);
		destroyStorageBuffer(m_pointSSBO);
		destroyUniformBuffer(m_dirUBO);
		destroyUniformBuffer(m_lightDataUBO);
	}

	std::vector<VkDescriptorPoolSize> LightDescriptorInterface::getRequiredPoolSizes()
	{
		std::vector<VkDescriptorPoolSize> poolSizes(3);

		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = r_FRAMES_IN_FLIGHT; // light data  ubo

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = r_FRAMES_IN_FLIGHT; // dir light ubo

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = r_FRAMES_IN_FLIGHT; // point light ssbo

		return poolSizes;
	}

	bool LightDescriptorInterface::allocateDescriptorSets(VkDescriptorPool descriptorPool)
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
			clz::log::error("vulkan could not allocate light descriptor sets");
			return false;
		}

		for (size_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			setHandleName(
				reinterpret_cast<uint64_t>(m_descriptorSets[i]),
				VK_OBJECT_TYPE_DESCRIPTOR_SET,
				("light descriptor [frame " + std::to_string(i) + "]").c_str()
			);
		}

		return true;
	}

	void LightDescriptorInterface::updateDescriptorSets()
	{
		for (size_t i = 0; i < r_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo lightDataInfo{};
			lightDataInfo.buffer = m_lightDataUBO.buffer;
			lightDataInfo.offset = m_lightDataUBO.offsets[i];
			lightDataInfo.range = m_lightDataUBO.uboSize;

			VkDescriptorBufferInfo dirLightInfo{};
			dirLightInfo.buffer = m_dirUBO.buffer;
			dirLightInfo.offset = m_dirUBO.offsets[i];
			dirLightInfo.range = m_dirUBO.uboSize;

			VkDescriptorBufferInfo pointLightInfo{};
			pointLightInfo.buffer = m_pointSSBO.buffer;
			pointLightInfo.offset = m_pointSSBO.offsets[i];
			pointLightInfo.range = m_pointSSBO.ssboSize;

			VkWriteDescriptorSet lightDataWrite{};
			lightDataWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			lightDataWrite.dstSet = m_descriptorSets[i];
			lightDataWrite.dstBinding = LIGHT_DATA_BIND_POINT;
			lightDataWrite.dstArrayElement = 0;
			lightDataWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			lightDataWrite.descriptorCount = 1;
			lightDataWrite.pBufferInfo = &lightDataInfo;

			VkWriteDescriptorSet dirLightWrite{};
			dirLightWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			dirLightWrite.dstSet = m_descriptorSets[i];
			dirLightWrite.dstBinding = DIR_LIGHT_BIND_POINT;
			dirLightWrite.dstArrayElement = 0;
			dirLightWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			dirLightWrite.descriptorCount = 1;
			dirLightWrite.pBufferInfo = &dirLightInfo;

			VkWriteDescriptorSet pointLightWrite{};
			pointLightWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			pointLightWrite.dstSet = m_descriptorSets[i];
			pointLightWrite.dstBinding = POINT_LIGHT_BIND_POINT;
			pointLightWrite.dstArrayElement = 0;
			pointLightWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			pointLightWrite.descriptorCount = 1;
			pointLightWrite.pBufferInfo = &pointLightInfo;

			const std::array<VkWriteDescriptorSet, 3> writes = {
				lightDataWrite, dirLightWrite, pointLightWrite
			};

			vkUpdateDescriptorSets(
				r_deviceContext.device,
				static_cast<uint32_t>(writes.size()),
				writes.data(),
				0,
				nullptr
			);
		}
	}

	void LightDescriptorInterface::update()
	{
		memcpy(m_dirUBO.mapped[r_currentFrame],
		       Lights.directionalLight.data(),
		       sizeof(DirectionalLight) * numDirectionalLights);

		auto& pointLightEntities = ecs::getEntitiesWithComponent<PointLightComponent>();
		const uint32_t activePointLights = pointLightEntities.size();
		const ShaderLightData lightData{
			.numPointLights = activePointLights,
			.numSpotLights = static_cast<uint32_t>(numSpotLights)
		};
		memcpy(m_lightDataUBO.mapped[r_currentFrame], &lightData, sizeof(ShaderLightData));

		for (auto& entity : pointLightEntities)
		{
			Lights.pointLights[ecs::getComponent<PointLightComponent>(entity).Id.value]
				.position =
					ecs::getComponent<ecs::TransformComponent>(entity).position;
		}
		memcpy(m_pointSSBO.mapped[r_currentFrame],
		       Lights.pointLights.data(),
		       sizeof(PointLight) * activePointLights
		);
	}
} // namespace clz::renderer
