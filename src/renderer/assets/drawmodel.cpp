#include "renderer/assets/drawmodel.hpp"

#include "math/worldtransform.hpp"

#include "ecs/componentmanager.hpp"
#include "ecs/components.hpp"

#include "renderer/shaderdata/bufferarray/vertexbuffer.hpp"
#include "renderer/shaderdata/bufferarray/indexbuffer.hpp"
#include "renderer/shaderdata/bufferarray/uvbuffer.hpp"

#include "renderer/vk_types.hpp"
#include "renderer/shaderdata/pushconstant/mainpipeline.hpp"

namespace clz::renderer
{
	void drawEntitiesMainPipeline(VkCommandBuffer commandBuffer)
	{
		const auto& entities =
			ecs::getEntitiesWithComponent<ecs::ModelComponent>();
		for (const auto& entity : entities)
		{
			const auto& modelComponent = ecs::getComponent<ecs::ModelComponent>(entity);
			const auto& transformComponent = ecs::getComponent<ecs::TransformComponent>(entity);

			for (const auto& meshID : Asset::r_modelLUT[modelComponent.modelID].meshes)
			{
				constexpr VkDeviceSize offsets[] = {0, 0};
				const VkBuffer buffers[] = {VBuffer::r_vertexBuffer, UVBuffer::r_uvBuffer};
				vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);
				vkCmdBindIndexBuffer(commandBuffer, IBuffer::r_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

				MainPC::vertexData.model = math::getModelMatrix(transformComponent.rotation,
					transformComponent.position, transformComponent.scale);
				vkCmdPushConstants(commandBuffer, r_pipelineContext.layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
					sizeof(MainPC::VertexData), &MainPC::vertexData);
				MainPC::fragmentData.textureID = mesh::baseMaterialLUT.texture[mesh::meshLUT.material[meshID]];
				vkCmdPushConstants(commandBuffer, r_pipelineContext.layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MainPC::VertexData),
					sizeof(MainPC::FragmentData), &MainPC::fragmentData);

				vkCmdDrawIndexed(commandBuffer, mesh::meshLUT.indexCount[meshID],
							1, mesh::meshLUT.firstIndex[meshID],
							mesh::meshLUT.baseVertex[meshID], 0);
			}
		}

	}
} // namespace clz::renderer