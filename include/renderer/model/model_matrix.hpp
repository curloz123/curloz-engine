/**
 * @file model_matrix.hpp
 * @author curl0z
 * @brief Manages model's matrices of models.
 */
#pragma once

#include "cstdint"
#include <vector>
#include "renderer/utility/ssbo.hpp"
#include "math/mat4x4.hpp"
#include "core/secure_types.hpp"

namespace clz::renderer
{
	/**
	 * @brief We store model matrices in manner of a range.
	 * Each range has a baseIndex which is the start index in
	 * global array of model matrices and count.
	 * Each matrice should correspond to a node, and
	 * is actually indexed by node only!!
	 */
	struct ModelMatrixRange
	{
		/// First Index
		/// Is used to check whether this range is valid
		/// via baseIndex.hasValue()
		SecureType_t<Types::U32> baseIndex;

		/// Count of matrices
		uint32_t count = 0;
	};

	class ModelMatrixManager
	{
	private:

		SSBO m_ssbo;

		/// CPU side stored model matrices. Useful for dirty flag n all.
		std::vector<math::mat4> m_cachedMatrices;

		std::vector<ModelMatrixRange> m_gaps;
		uint32_t m_nextFreshIndex = 0;
		uint32_t m_capacity;

		/**
		 * @brief Whenever any matrix range is freed,
		 * It is passed into m_gaps, which stores gaps in inc order by size.
		 * @param rMatrixRange const reference to matrix range
		 * @note This function does not make the handle null.
		 */
		void insertGap(const ModelMatrixRange& rMatrixRange);

	public:
		/// model matrix bind point in shaders.
		static inline constexpr uint32_t MODEL_MATRIX_BIND_POINT = 1;

		/// @brief Initializes matrix manager and all its resources.
		bool initialize(const uint32_t modelMatrixCount = 8192);

		/// @brief Destroys matrix manager.
		/// @param explicitDestroy To destroy immediately!!?? No by default
		/// engine will clean stuff up later.
		void destroy(const bool explicitDestroy = false);

		/// @brief Allocates matrices range.
		/// @param numMatrices Number of matrices to allocate.
		/// @param reference to matrix range.
		/// @return true on succesful allocation, false otherwise.
		bool allocateRange(
			const uint32_t numMatrices,
			ModelMatrixRange& rMatrixRange
		);

		/// @brief Frees a range.
		/// @param rMatrixRange range to free.
		/// @note range is null after this func.
		void freeRange(ModelMatrixRange& rMatrixRange);

		/// @brief Sets a matrix for GPU.
		/// @param matrixRange actual matrices range handle.
		/// @param matrixOffset This is basically node's index.
		/// @param tempMatrix Actual model matrix to set.
		/// @note tempMatrix must be an rvalue reference.
		void setMatrix(
			const ModelMatrixRange& matrixRange,
			const uint32_t matrixOffset,
			math::mat4&& tempMatrix
		);

		const math::mat4& getMatrix(
			const ModelMatrixRange& range,
			const uint32_t offset
		) const
		{
#ifdef CLZ_ENABLE_CHECKS /*****************************************************/
			CLZ_ASSERT(
				range.baseIndex.hasValue() &&
					range.baseIndex.value() + offset < m_cachedMatrices.size(),
				"Attempt to get matrix of an out-of-bound/null range"
			);
			CLZ_ASSERT(
				offset < range.count,
				"Attempt to get matrix with offset greater than count"
			);
#endif /***********************************************************************/

			return m_cachedMatrices[range.baseIndex.value() + offset];
		}

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
