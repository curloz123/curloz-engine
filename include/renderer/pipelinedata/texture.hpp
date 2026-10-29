#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "../vk_types.hpp"
#include "renderer/context/context.hpp"

namespace clz::renderer
{

	class TextureDescriptorInterface
	{
	private:
		VkDescriptorSetLayout m_descriptorLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_descriptorSets;

	public:
		static inline constexpr uint32_t TEXTURE_SET_POINT = 1;
		static inline constexpr uint32_t TEXTURE_BIND_POINT = 0;
		static inline constexpr uint32_t SAMPLER_BIND_POINT = 1;
		static inline constexpr uint32_t MATERIAL_BIND_POINT = 2;

		bool initialize();

		void destroy();

		std::vector<VkDescriptorPoolSize> getRequiredPoolSizes();

		bool allocateDescriptorSets(VkDescriptorPool descriptorPool);

		/// Must be updated by Model system
		void updateDescriptorSets();

		void bindDescriptor(VkCommandBuffer commandBuffer)
		{
			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				r_pipelineContext.layout,
				TEXTURE_SET_POINT,
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
				layoutArray.size() > TEXTURE_SET_POINT,
				"Cannot populate an array less than set point"
			);
#endif /***********************************************************************/

			layoutArray[TEXTURE_SET_POINT] = m_descriptorLayout;
		}
	};
} // namespace clz::renderer
