#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include "renderer/vk_types.hpp"

namespace clz::renderer
{
	/**
	 * @brief Post-process pass descriptor resources: render target, bloom
	 * sample/mip chain, bloomed composite, tonemap, and post-tonemap images.
	 * Owned/written only by the post-process system, after it's initialized.
	 */
	class PostProcessDescriptorInterface
	{
	private:
		static inline constexpr uint32_t POST_PROCESS_SET_POINT = 3;
		static inline constexpr uint32_t RENDER_TARGET_IMAGE_BIND_POINT = 0;
		static inline constexpr uint32_t BLOOM_SAMPLE_IMAGE_BIND_POINT = 1;
		static inline constexpr uint32_t BLOOM_MIP_IMAGES_BIND_POINT = 2;
		static inline constexpr uint32_t BLOOMED_IMAGE_BIND_POINT = 3;
		static inline constexpr uint32_t TONEMAP_IMAGE_BIND_POINT = 4;
		static inline constexpr uint32_t POST_TONEMAP_IMAGE_BIND_POINT = 5;

		VkDescriptorSetLayout m_descriptorLayout = VK_NULL_HANDLE;
		std::array<VkDescriptorSet, r_FRAMES_IN_FLIGHT> m_descriptorSets;

	public:
		bool initialize();
		void destroy();

		std::vector<VkDescriptorPoolSize> getRequiredPoolSizes();

		bool allocateDescriptorSets(VkDescriptorPool descriptorPool);

		/// Must be updated by post_process system
		void updateDescriptorSets();

		void bindDescriptor(VkCommandBuffer commandBuffer, const PipelineContext& pipelineContext)
		{
			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineContext.layout,
				POST_PROCESS_SET_POINT,
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
				layoutArray.size() > POST_PROCESS_SET_POINT,
				"Cannot populate an array less than set point"
			);
#endif /***********************************************************************/

			layoutArray[POST_PROCESS_SET_POINT] = m_descriptorLayout;
		}
	};
} // namespace clz::renderer
