#include "renderer/model/model_material.hpp"
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

         	return materialId;
    	}
	void ModelMaterialManager::freeMaterial(ModelMaterialId& id)
      	{
		m_freeIndices.push_back(id);
		id.nullify();
       	}
}
