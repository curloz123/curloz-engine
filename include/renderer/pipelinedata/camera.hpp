/**
 * @file camera.hpp
 * @author curl0z
 * @brief Camera descriptor resources and everything is stored here
 */
#pragma once

#include "math/mat4x4.hpp"
#include "math/vec4.hpp"
#include "renderer/utility/ubo.hpp"
#include <memory.h>
#include <vulkan/vulkan_core.h>
#include "../vk_types.hpp"

namespace clz::renderer
{
	struct CameraShaderUBO
	{
		math::mat4 projection;
		math::mat4 view;
		math::vec4 cameraPos;
	};

	class CameraDescriptorInterface
	{
	private:
		VkDescriptorSetLayout m_descriptorLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_descriptorSets;

		UBO m_ubo;

	public:
		static inline constexpr uint32_t CAMERA_SET_POINT = 0;
		static inline constexpr uint32_t CAMERA_BIND_POINT = 0;
		static inline constexpr uint32_t MODEL_MATRIX_BIND_POINT = 1;

		bool initialize();

		void destroy();

		std::vector<VkDescriptorPoolSize> getRequiredPoolSize();

		bool allocateDescriptorSets(VkDescriptorPool descriptorPool);

		void updateVPDescriptorSets();
		void updateMDescriptorSets();

		void update(const CameraShaderUBO& cameraShaderUBO)
		{
			memcpy(m_ubo.mapped[r_currentFrame], &cameraShaderUBO, sizeof(CameraShaderUBO));
		}

		void bindDescriptor(VkCommandBuffer commandBuffer)
		{
			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				r_pipelineContext.layout,
				CAMERA_SET_POINT,
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
				layoutArray.size() > CAMERA_SET_POINT,
				"Cannot populate an array less than set point"
			);
#endif /***********************************************************************/

			layoutArray[CAMERA_SET_POINT] = m_descriptorLayout;
		}
	};
} // namespace clz::renderer
