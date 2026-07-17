#include "renderer/entitydata/rendermodel.hpp"
#include "core/enginestate.hpp"
#include "math/worldtransform.hpp"
#include "renderer/entitydata/indexbuffer.hpp"
#include "renderer/entitydata/uvbuffer.hpp"
#include "renderer/entitydata/vertexbuffer.hpp"
#include "renderer/pipelinedata/pushconstants.hpp"
#include "renderer/vk_types.hpp"
#include "scene/entity/componentmanager.hpp"
#include "scene/entity/components.hpp"

namespace clz::renderer
{
	void renderEntities(VkCommandBuffer commandBuffer)
	{
		const auto& entities = ecs::getEntitiesWithComponent<ecs::ModelComponent>();
		for (const auto& entity : entities)
		{
			const auto name = ecs::getEntityName(entity);
			const auto& modelComponent = ecs::getComponent<ecs::ModelComponent>(entity);
			math::mat4 model;
#ifdef CLZ_ENABLE_EDITOR
			if (state::g_engineState == state::EngineState::Editor)
			{
				const auto& transformComponent = ecs::getComponent<ecs::EditorTransformComponent>(entity);
				model = math::getModelMatrix(transformComponent.rotation, transformComponent.position, transformComponent.scale);
			}
			else
			{
				const auto& transformComponent = ecs::getComponent<ecs::TransformComponent>(entity);
				model = math::getModelMatrix(transformComponent.rotation, transformComponent.position, transformComponent.scale);
			}
#else
			const auto& transformComponent = ecs::getComponent<ecs::TransformComponent>(entity);
			model = math::getModelMatrix(transformComponent.rotation, transformComponent.position, transformComponent.scale);
#endif

			renderEntity(commandBuffer, modelComponent.modelID, model);
		}
	}

	void renderEntity(VkCommandBuffer commandBuffer, const ModelID modelID, const math::mat4& model)
	{
		for (const auto& meshID : Asset::r_modelLUT[modelID].meshes)
		{
			const VkDeviceSize offsets[] = {0, 0};
			const VkBuffer buffers[] = {r_vertexBuffer, r_uvBuffer};
			vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, r_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			ModelDataPC pc = {.modelMatrix = model, .textureID = mesh::baseMaterialLUT.texture[mesh::meshLUT.material[meshID]]};
			vkCmdPushConstants(commandBuffer, r_pipelineContext.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelDataPC), &pc);

			vkCmdDrawIndexed(commandBuffer, mesh::meshLUT.indexCount[meshID], 1, mesh::meshLUT.firstIndex[meshID],
					 mesh::meshLUT.baseVertex[meshID], 0);
		}
	}
} // namespace clz::renderer
