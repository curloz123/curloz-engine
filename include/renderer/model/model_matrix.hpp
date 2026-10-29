#pragma once

#include "cstdint"
#include <vector>
#include "core/id_interface.hpp"
#include "renderer/utility/ssbo.hpp"
#include "math/mat4x4.hpp"

namespace clz::renderer
{
	class ModelMatrixId : public IdInterface<uint32_t>{};

	class ModelMatrixManager
	{
	private:
		SSBO m_ssbo;
		std::vector<ModelMatrixId> m_freeIndices;
		uint32_t m_nextFreshIndex = 0;
		uint32_t m_capacity;

	public:
		static inline constexpr uint32_t MODEL_MATRIX_BIND_POINT = 1;

		bool initialize(const uint32_t modelMatrixCount = 8192);
		void destroy(const bool explicitDestroy = false);

	    	ModelMatrixId allocate();
		void free(ModelMatrixId& id);

		void setMatrix(
			const ModelMatrixId matrixId,
			const math::mat4& tempMatrix
		);

		const SSBO& getStorageBuffer() const
		{
			return m_ssbo;
		}

		uint32_t getMatrixArrayLimit() const
		{
			return m_nextFreshIndex;
		}
	};
}
