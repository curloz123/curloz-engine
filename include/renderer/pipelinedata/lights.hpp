#pragma once

#include "renderer/utility/ssbo.hpp"
#include "renderer/utility/ubo.hpp"
#include "renderer/vk_types.hpp"
#include <vector>

namespace clz::renderer
{
	/**
	 * @brief Lighting information required by the shaders.
	 * Contains the number of point and spot lights currently active in the scene.
	 */
	struct ShaderLightData
	{
		uint32_t numPointLights;
		uint32_t numSpotLights;
	};

	class LightDescriptorInterface
	{
	private:
		static inline constexpr uint32_t LIGHT_SET_POINT = 2;
		static inline constexpr uint32_t LIGHT_DATA_BIND_POINT = 0;
		static inline constexpr uint32_t DIR_LIGHT_BIND_POINT = 1;
		static inline constexpr uint32_t POINT_LIGHT_BIND_POINT = 2;
		static inline constexpr uint32_t SPOT_LIGHT_BIND_POINT = 3;

		VkDescriptorSetLayout m_descriptorLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_descriptorSets;

		UBO m_lightDataUBO;
		UBO m_dirUBO;
		SSBO m_pointSSBO;
		SSBO m_spotSSBO;

	public:
		bool initialize();
		void destroy();

		std::vector<VkDescriptorPoolSize> getRequiredPoolSizes();

		bool allocateDescriptorSets(VkDescriptorPool descriptorPool);

		void updateDescriptorSets();

		void update();

		void bindDescriptor(VkCommandBuffer commandBuffer)
		{
			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				r_pipelineContext.layout,
				LIGHT_SET_POINT,
				1,
				&m_descriptorSets[r_currentFrame],
				0,
				nullptr
			);
		}

		VkDescriptorSetLayout getLayout() const
		{
			return m_descriptorLayout;
		}
		void populateLayoutArray(
			std::vector<VkDescriptorSetLayout>& layoutArray
		) const
		{

#ifdef CLZ_ENABLE_CHECKS /*****************************************************/
			CLZ_ASSERT(
				layoutArray.size() > LIGHT_SET_POINT,
				"Cannot populate an array less than set point"
			);
#endif /***********************************************************************/

			layoutArray[LIGHT_SET_POINT] = m_descriptorLayout;
		}
	};
} // namespace clz::renderer
