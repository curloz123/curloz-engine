#include "renderer/model/model_matrix.hpp"
#include "core/logs.hpp"
#include "math/mat4x4.hpp"
#include "renderer/utility/ssbo.hpp"
#include <algorithm>

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
		m_gaps.clear();
		return true;
	}

	void ModelMatrixManager::destroy(const bool explicitDestroy)
	{
		destroyStorageBuffer(m_ssbo, explicitDestroy);
		m_gaps.clear();
		m_nextFreshIndex = 0;
		m_capacity = 0;
	}

	void ModelMatrixManager::insertGap(
		const ModelMatrixRange& rMatrixRange
	)
	{
		if (rMatrixRange.count == 0)
			return;

		const auto pos = std::lower_bound(
			m_gaps.begin(),
			m_gaps.end(),
			rMatrixRange.count,
			[](const ModelMatrixRange& r, const uint32_t c) {
				return r.count < c;
			}
		);
		m_gaps.insert(pos, rMatrixRange);
	}

	bool ModelMatrixManager::allocateRange(
		const uint32_t numMatrices,
		ModelMatrixRange& rMatrixRange
	)
	{
		if (!m_gaps.empty() && m_gaps.back().count >= numMatrices)
		{
			const auto it = std::lower_bound(
				m_gaps.begin(),
				m_gaps.end(),
				numMatrices,
				[](const ModelMatrixRange& r, const uint32_t c) {
					return r.count < c;
				}
			);

			rMatrixRange = ModelMatrixRange{it->baseIndex, numMatrices};

			it->baseIndex = it->baseIndex.value() + numMatrices;
			it->count -= numMatrices;

			if (it->count == 0)
			{
				m_gaps.erase(it);
				return true;
			}

			if (it == m_gaps.begin())
				return true;
			else
			{
				const auto newPos = std::lower_bound(
					m_gaps.begin(),
					it,
					it->count,
					[](const ModelMatrixRange& r, const uint32_t c) {
						return r.count < c;
					}
				);
				if (newPos != it)
				{
					std::reverse(newPos, it);
					std::reverse(newPos, it + 1);
				}

				return true;
			}
		}

		if(m_nextFreshIndex + numMatrices > m_capacity)
		{
			clz::log::error("Matrix manager has reached its limit");
			return false;
		}
		rMatrixRange.baseIndex = m_nextFreshIndex;
		rMatrixRange.count = numMatrices;

		m_nextFreshIndex += numMatrices;
		m_cachedMatrices.resize(m_nextFreshIndex);

		return true;
	}

	void ModelMatrixManager::freeRange(ModelMatrixRange& rMatrixRange)
	{
		if (rMatrixRange.count == 0)
		{
			rMatrixRange.baseIndex = UINT32_MAX;
			return;
		}

		auto freedBase  = rMatrixRange.baseIndex;
		uint32_t freedCount = rMatrixRange.count;

		const auto rightIt = std::find_if(
			m_gaps.begin(), m_gaps.end(),
			[rightEnd = freedBase.value() + freedCount](const ModelMatrixRange& r) {
				return r.baseIndex.value() == rightEnd;
			}
		);
		if (rightIt != m_gaps.end())
		{
			freedCount += rightIt->count;
			m_gaps.erase(rightIt);
		}

		const auto leftIt = std::find_if(
			m_gaps.begin(), m_gaps.end(),
			[leftEnd = freedBase](const ModelMatrixRange& r) {
				return r.baseIndex.value() + r.count == leftEnd.value();
			}
		);
		if (leftIt != m_gaps.end())
		{
			freedBase = leftIt->baseIndex;
			freedCount += leftIt->count;
			m_gaps.erase(leftIt);
		}

		insertGap(ModelMatrixRange{freedBase, freedCount});
		rMatrixRange.baseIndex.makeNull();
		rMatrixRange.count = 0;
	}

	void ModelMatrixManager::setMatrix(
		const ModelMatrixRange& matrixRange,
		const uint32_t matrixOffset,
		math::mat4&& tempMatrix
	)
	{
#ifdef CLZ_ENABLE_CHECKS /*****************************************************/
		CLZ_ASSERT(
			matrixRange.baseIndex.hasValue() &&
				matrixRange.baseIndex.value() + matrixOffset < m_cachedMatrices.size(),
			"Attempt to get matrix of an out-of-bound/null range"
		);
		CLZ_ASSERT(
			matrixOffset < matrixRange.count,
			"Attempt to set matrix with offset greater than count"
		);
#endif /***********************************************************************/

		copyDataToSSBO(
			m_ssbo,
			&tempMatrix,
			sizeof(math::mat4),
			(matrixRange.baseIndex.value() + matrixOffset) * sizeof(math::mat4)
		);

		m_cachedMatrices[matrixRange.baseIndex.value() + matrixOffset] = std::move(tempMatrix);
	}
}
