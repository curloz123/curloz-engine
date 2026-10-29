#include "renderer/model/model_matrix.hpp"
#include "core/logs.hpp"
#include "core/assert.hpp"
#include "math/mat4x4.hpp"
#include "renderer/utility/ssbo.hpp"

namespace clz::renderer
{
	bool ModelMatrixManager::initialize(const uint32_t modelMatrixCount)
	{
		const StorageBufferCreateInfo ssboCI{
			.size = sizeof(math::mat4) * modelMatrixCount,
			.bindPoint = MODEL_MATRIX_BIND_POINT,
		};
		if (!createStorageBuffer(ssboCI, m_ssbo, "model matrix"))
		{
			clz::log::error(
				"Could not allocate storage buffer for model matrix",
				true
			);
			return false;
		}

		m_capacity = modelMatrixCount;
		m_nextFreshIndex = 0;
		m_freeIndices.clear();
		return true;
	}
	void ModelMatrixManager::destroy(const bool explicitDestroy)
	{
		destroyStorageBuffer(m_ssbo, explicitDestroy);
		m_freeIndices.clear();
		m_nextFreshIndex = 0;
		m_capacity = 0;
	}
    	ModelMatrixId ModelMatrixManager::allocate()
     	{
        	if (!m_freeIndices.empty())
        	{
         		auto idx = m_freeIndices.back();
        	    	m_freeIndices.pop_back();
         	    	return idx;
        	}

         	CLZ_ASSERT(
          		m_nextFreshIndex < m_capacity,
            		"model matrix SSBO full"
          	);
          	ModelMatrixId modelMatrixId(m_nextFreshIndex++);
           	return modelMatrixId;
      	}

	void ModelMatrixManager::free(ModelMatrixId& id)
      	{
		m_freeIndices.push_back(id);
		id.nullify();
       	}


        void ModelMatrixManager::setMatrix(
        	const ModelMatrixId matrixId,
         	const math::mat4& tempMatrix
        )
        {
        	copyDataToSSBO(
         		m_ssbo,
           		&tempMatrix,
           		sizeof(math::mat4),
             		matrixId.getId() * sizeof(math::mat4)
         	);
        }
}
