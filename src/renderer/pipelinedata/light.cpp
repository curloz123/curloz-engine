/**
 * @file light.cpp
 * @author curl0z
 * @brief Light descriptor implementation file
 */
#include "core/logs.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "renderer/pipelinedata/lights.hpp"
#include "renderer/rendercomponent.hpp"
#include "renderer/utility/descriptor.hpp"
#include "renderer/vk_types.hpp"
#include <string>

namespace clz::renderer
{
	/// @copydoc initLightDescriptor
	bool initLightDescriptor()
	{
		/// --- 1. Creating storages ---

		if (!createUniformBuffer(lightDataUBO, lightDataUBOMemory, "light data ubo"))
		{
			clz::log::error("could not create light data ubo buffer");
			return false;
		}

		if (!createUniformBuffer(dirUBO, dirUBOMemory, "directional light ubo"))
		{
			clz::log::error("could not create directional light uniform buffer");
			return false;
		}

		if (!createStorageBuffer(pointSSBO, pointSSBOMemory, "point light ssbo"))
		{
			clz::log::error("could not create point light ssbo buffer");
			return false;
		}

		/// --- 2. creating layout ---
		constexpr uint8_t numLightBindPoints = 3;
		constexpr std::array<uint32_t, numLightBindPoints> bindPoints = {
			LIGHT_DATA_BIND_POINT,
			DIR_LIGHT_BIND_POINT,
			POINT_LIGHT_BIND_POINT
		};
		constexpr std::array<VkDescriptorType, numLightBindPoints> descriptorTypes = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		};
		constexpr std::array<uint32_t, numLightBindPoints> descriptorCounts = {1, 1, 1};
		constexpr std::array<VkShaderStageFlags, numLightBindPoints> shaderStages = {
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT
		};
		if (!createDescriptorLayout(
			    lightDescriptorLayout,
			    bindPoints,
			    descriptorTypes,
			    descriptorCounts,
			    shaderStages,
			    "camera ubo layout"
		    ))
		{
			clz::log::error("Could not create light descriptor layout!");
			return false;
		}
		return true;
	}

	/// @copydoc updateLightDescriptor
	void updateLightDescriptor()
	{
		const ShaderLightData lightData{
			.numPointLights = static_cast<uint32_t>(Lights.pointLights.size()),
			.numSpotLights = static_cast<uint32_t>(Lights.spotLights.size())
		};
		memcpy(lightDataUBO.mapped[r_currentFrame], &lightData, sizeof(ShaderLightData));

		memcpy(dirUBO.mapped[r_currentFrame],
		       Lights.directionalLight.data(),
		       sizeof(DirectionalLight) * Lights.directionalLight.size());

		auto& pointLightEntities = ecs::getEntitiesWithComponent<PointLightComponent>();
		for (auto& entity : pointLightEntities)
		{
			Lights.pointLights[ecs::getComponent<PointLightComponent>(entity).Id.value]
				.position =
				ecs::getComponent<ecs::TransformComponent>(entity).position;
		}
		memcpy(pointSSBO.mapped[r_currentFrame],
		       Lights.pointLights.data(),
		       sizeof(PointLight) * Lights.pointLights.size());
	}

	/// @copydoc destroyLightDescriptor
	void destroyLightDescriptor()
	{
		destroyDescriptorSetLayout(lightDescriptorLayout);
		destroyStorageBuffer(pointSSBO, pointSSBOMemory);
		destroyUniformBuffer(dirUBO, dirUBOMemory);
		destroyUniformBuffer(lightDataUBO, lightDataUBOMemory);
	}
} // namespace clz::renderer