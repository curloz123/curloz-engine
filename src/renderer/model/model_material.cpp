#include "renderer/model/model_material.hpp"
#include "renderer/entitydata/texture.hpp"
#include "renderer/utility/ssbo.hpp"
#include "core/logs.hpp"
#include "renderer/pipelinedata/pipeline_data.hpp"

namespace clz::renderer
{
	bool ModelMaterialManager::initialize(const uint32_t modelMaterialCount)
	{
		const StorageBufferCreateInfo ssboCI{
			.size = sizeof(ModelMaterial) * modelMaterialCount,
			.bindPoint = TextureDescriptor.MATERIAL_BIND_POINT,
		};
		if (!createStorageBuffer(ssboCI, m_ssbo, "model matrix"))
		{
			clz::log::error(
				"Could not allocate storage buffer for model matrix",
				true
			);
			return false;
		}

		m_capacity = modelMaterialCount;
		m_nextFreshIndex = 0;
		m_freeIndices.clear();
		return true;
	}
	void ModelMaterialManager::destroy(const bool explicitDestroy)
	{
		destroyStorageBuffer(m_ssbo, explicitDestroy);
		m_freeIndices.clear();
		m_nextFreshIndex = 0;
		m_capacity = 0;
	}

	ModelMaterialId ModelMaterialManager::submitMaterial(
		const ModelMaterial& modelMaterial
	)
  	{
		ModelMaterialId materialId;
     		if (!m_freeIndices.empty())
     		{
      			materialId = m_freeIndices.back();
     		    	m_freeIndices.pop_back();
     		}
     		else
       		{
       			CLZ_ASSERT(m_nextFreshIndex < m_capacity, "material matrix SSBO full");
          		materialId = ModelMaterialId(m_nextFreshIndex++);
       		}

       		copyDataToSSBO(
         		m_ssbo,
           		&modelMaterial,
             		sizeof(ModelMaterial),
               		sizeof(ModelMaterial) * materialId.getId()
         	);

         	if (materialId.getId() >= m_cachedMaterials.size())
           		m_cachedMaterials.resize(materialId.getId() + 1);

          	m_cachedMaterials[materialId.getId()] = modelMaterial;

         	return materialId;
    	}
    	void ModelMaterialManager::freeMaterial(
     		ModelMaterialId& id
     	)
	{
		constexpr bool immediateDestroy = true;
		auto& material = m_cachedMaterials[id.getId()];
		const auto defaultSamplerId = TextureManager.getDefaultSamplerId();

		auto freeTexture = [&](const TextureId& textureId)
		{
		   	if (!textureId.isNull())
		        	TextureManager.destroyTexture(textureId, immediateDestroy);
		};
		auto freeSampler = [&](const SamplerId& samplerId)
		{
		    	if (!samplerId.isNull() &&
				samplerId.getId() != defaultSamplerId.getId()
			)
		        	TextureManager.destroyTextureSampler(samplerId);
		};

		freeTexture(material.baseTextureId);
		freeTexture(material.metallicRoughnessTextureId);
		freeTexture(material.emissiveTextureId);
		freeTexture(material.normalTextureId);
		freeTexture(material.specularColorTextureId);
		freeTexture(material.specularTextureId);

		freeSampler(material.baseSamplerId);
		freeSampler(material.metallicRoughnessSamplerId);
		freeSampler(material.emissiveSamplerId);
		freeSampler(material.normalSamplerId);
		freeSampler(material.specularColorSamplerId);
		freeSampler(material.specularSamplerId);

		m_freeIndices.push_back(id);
		id.nullify();
	}
}
