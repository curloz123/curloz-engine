#pragma once

#include <vulkan/vulkan.h>
#include "../arena.hpp"
#include "../vk_types.hpp"
#include "core/logs.hpp"
#include <memory.h>
#include <vulkan/vulkan_core.h>
#include "math/vec3.hpp"
#include "math/vec2.hpp"
#include "math/vec4.hpp"

namespace clz::renderer
{
	typedef struct VertexRange
	{
		uint32_t baseIndex;
		uint32_t vertexCount;
	} VRange;
	typedef struct IndexRange
	{
		uint32_t firstIndex;
		uint32_t indexCount;
	} IRange;
	template <typename MeshStruct>
	class VertexBufferInterface
	{
	private:
		VkBuffer m_buffer = VK_NULL_HANDLE;
		clzVertexBufferMemory m_memory;

		VkDeviceSize m_newVertexStartIndex;
		std::vector<VertexRange> m_gaps;
	public:
		bool initialize(const VkDeviceSize totalSize)
		{
			const auto& device = r_deviceContext.device;
			const VkBufferCreateInfo bufferCI{
				.sType 		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext 		= nullptr,
				.flags 		= 0,
				.size  		= totalSize,
				.usage 		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
							VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				.sharingMode 	= VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices   = nullptr
			};
			if (vkCreateBuffer(
				device,
				&bufferCI,
				nullptr,
				&m_buffer
			) != VK_SUCCESS)
			{
				clz::log::error(
					"Vulkan failed to create a vertex buffer",
					true
				);
				return false;
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(
				device,
				m_buffer,
				&memRequirements
			);

			const VBufferMemAllocInfo allocInfo{
				.size = totalSize,
				.alignment = memRequirements.alignment
			};

			if (!allocateVertexBufferMemory(allocInfo, m_memory))
			{
				clz::log::error("Error while trying to carve memory", true);
				return false;
			}

			bindVertexBufferToMemory(m_buffer, m_memory);
			m_newVertexStartIndex = 0;
			return true;
		}

		void destroy(const bool explicitDestroy = false)
		{
			vkDestroyBuffer(r_deviceContext.device, m_buffer, nullptr);
			m_gaps.clear();
			m_newVertexStartIndex = 0;
			if (explicitDestroy)
				freeVertexBufferMemory(m_memory);
		}

		void bindVertexBuffer(VkCommandBuffer commandBuffer) const
		{
			constexpr std::array<VkDeviceSize, 1> offsets = {0};

			vkCmdBindVertexBuffers(
				commandBuffer,
				0,
				1,
				&m_buffer,
				offsets.data()
			);
		}

		void insertGap(
			const uint32_t baseIndex,
			const uint32_t vertexCount
		)
		{
			if (vertexCount == 0)
				return;

			const auto pos = std::lower_bound(
				m_gaps.begin(),
				m_gaps.end(),
				vertexCount,
				[](const VertexRange& r, const uint32_t count) {
					return r.vertexCount < count;
				}
			);
			m_gaps.insert(pos, VertexRange{baseIndex, vertexCount});
		}

		/**
		 * @brief Records commands that submits data of vertices
		 * to this vertex buffer on GPU. This is the function that's gonna be
		 * called per-model.
		 * @note Remember: multiple buffers, sharing same memory.
		 * @param vertices Array that holds all vertex data. Must not be empty.
		 * @param commandBuffer Active recording command buffer.
		 * @param stagingBuffer Temporary staging buffer that'll hold temp data.
		 * @param stagingBufferMemoryOffset Offset of buffer in memory.
		 * @param stagingBufferMemory Memory of staging buffer.
		 * @note Must be a part of active command buffer operation.
		 * @warn Make sure to destroy the staging buffers and the memory only after
		 * submittion has been completed.
		 * @note Is not thread-safe. Don't trynna multithread it.
		 * @return VertexRange that holds the baseVertex in vertex buffer array, and number of vertices.
		 */
		[[nodiscard]] std::tuple<VertexRange, VkBufferCopy> recordVerticesSubmission(
			const std::span<MeshStruct> vertices,
			VkCommandBuffer commandBuffer,
			VkBuffer stagingBuffer,
			VkDeviceSize stagingBufferMemoryOffset,
			VkDeviceMemory stagingBufferMemory
		)
		{

#ifdef CLZ_ENABLE_CHECKS /*****************************************/
			CLZ_ASSERT(
				!vertices.empty(),
				"Attempt to register empty vertices array"
			);
			CLZ_ASSERT(
				stagingBufferMemory != VK_NULL_HANDLE,
				"null command buffer passed while registering vertices"
			);
			CLZ_ASSERT(
				commandBuffer != VK_NULL_HANDLE,
				"null command buffer passed"
					" while registering vertices"
			);
			CLZ_ASSERT(
				stagingBuffer != VK_NULL_HANDLE,
				"null buffer passed"
					" while registering vertices"
			);
#endif /***********************************************************/


			const auto& device = r_deviceContext.device;
			const VkDeviceSize bufferSize =
				sizeof(vertices[0]) * vertices.size();

			void* data;
			vkMapMemory(
				device,
				stagingBufferMemory,
				stagingBufferMemoryOffset,
				bufferSize,
				0,
				&data
			);
			memcpy(data, vertices.data(), bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			const auto vertexCount = static_cast<uint32_t>(vertices.size());

			auto retrieveRange = [&](){
				VertexRange outRange;
				if (!m_gaps.empty() && m_gaps.back().vertexCount >= vertexCount)
				{
					const auto it = std::lower_bound(
						m_gaps.begin(),
						m_gaps.end(),
						vertexCount,
						[](const VertexRange& r, const uint32_t count) {
							return r.vertexCount < count;
						}
					);

					outRange.baseIndex   = it->baseIndex;
					outRange.vertexCount = vertexCount;

					it->baseIndex += vertexCount;
					it->vertexCount -= vertexCount;

					if (it->vertexCount == 0)
					{
						m_gaps.erase(it);
						return outRange;
					}

					if (it == m_gaps.begin())
						return outRange;

					const auto newPos = std::lower_bound(
						m_gaps.begin(),
						it,
						it->vertexCount,
						[](const VertexRange& r, const uint32_t count) {
							return r.vertexCount < count;
						}
					);
					if (newPos != it)
					{
						std::reverse(newPos, it);
						std::reverse(newPos, it + 1);
					}

					return outRange;
				}

				outRange.baseIndex   = m_newVertexStartIndex;
				outRange.vertexCount = vertexCount;
				m_newVertexStartIndex += vertexCount;

				return outRange;

			};

			VertexRange vertexRange = retrieveRange();

			VkBufferCopy copyRegion{};
			copyRegion.size = bufferSize;
			copyRegion.srcOffset = stagingBufferMemoryOffset;
			copyRegion.dstOffset = vertexRange.baseIndex * sizeof(vertices[0]);

			return std::make_tuple(vertexRange, copyRegion);
		}

		void flushCopies(
			const std::span<VkBufferCopy> copyRegions, 
			VkCommandBuffer commandBuffer, 
			VkBuffer stagingBuffer
		)
		{
			if (copyRegions.empty())
				return;

			vkCmdCopyBuffer(
				commandBuffer, 
				stagingBuffer, 
				m_buffer, 
				static_cast<uint32_t>(copyRegions.size()), 
				copyRegions.data()
			);
		}

		void freeVertices(VertexRange& range)
		{

#ifdef CLZ_ENABLE_CHECKS /*******************************************/
			CLZ_ASSERT(
				range.vertexCount != 0,
				"freeing a null/empty vertex range"
			);
#endif /*************************************************************/

			uint32_t freedBase  = range.baseIndex;
			uint32_t freedCount = range.vertexCount;

			const auto rightIt = std::ranges::find_if(
				m_gaps,
				[rightEnd = freedBase + freedCount](const VertexRange& r) {
					return r.baseIndex == rightEnd;
				}
			);
			if (rightIt != m_gaps.end())
			{
				freedCount += rightIt->vertexCount;
				m_gaps.erase(rightIt);
			}

			const auto leftIt = std::ranges::find_if(
				m_gaps,
				[leftEnd = freedBase](const VertexRange& r) {
					return r.baseIndex + r.vertexCount == leftEnd;
				}
			);
			if (leftIt != m_gaps.end())
			{
				freedBase = leftIt->baseIndex;
				freedCount += leftIt->vertexCount;
				m_gaps.erase(leftIt);
			}

			insertGap(freedBase, freedCount);
		}
	};


	class IndexBufferInterface
	{
	private:
		VkBuffer m_buffer = VK_NULL_HANDLE;
		clzIndexBufferMemory m_memory;

		VkDeviceSize m_newIndiceStartIndex;
		std::vector<IndexRange> m_gaps;
	public:
		bool initialize(const VkDeviceSize totalSize)
		{
			const auto& device = r_deviceContext.device;
			const VkBufferCreateInfo bufferCI{
				.sType 		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext 		= nullptr,
				.flags 		= 0,
				.size  		= totalSize,
				.usage 		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | 
							VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				.sharingMode 	= VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices   = nullptr
			};
			if (vkCreateBuffer(
				device,
				&bufferCI,
				nullptr,
				&m_buffer
			) != VK_SUCCESS)
			{
				clz::log::error(
					"Vulkan failed to create a vertex buffer",
					true
				);
				return false;
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(
				device,
				m_buffer,
				&memRequirements
			);

			const IBufferMemAllocInfo allocInfo{
				.size = totalSize,
				.alignment = memRequirements.alignment
			};

			if (!allocateIndexBufferMemory(allocInfo, m_memory))
			{
				clz::log::error("Error while trying to carve memory", true);
				return false;
			}

			bindIndexBufferToMemory(m_buffer, m_memory);
			m_newIndiceStartIndex = 0;
			return true;
		}

		void destroy(const bool explicitDestroy = false)
		{
			vkDestroyBuffer(r_deviceContext.device, m_buffer, nullptr);
			m_gaps.clear();
			m_newIndiceStartIndex = 0;
			if (explicitDestroy)
				freeIndexBufferMemory(m_memory);
		}

		void bindIndexBuffer(VkCommandBuffer commandBuffer) const
		{
			vkCmdBindIndexBuffer(
				commandBuffer,
				m_buffer,
				0,
				VK_INDEX_TYPE_UINT32
			);
		}

		void insertGap(
			const uint32_t firstIndex,
			const uint32_t indexCount
		)
		{
			if (indexCount == 0)
				return;

			const auto pos = std::lower_bound(
				m_gaps.begin(),
				m_gaps.end(),
				indexCount,
				[](const IndexRange& r, const uint32_t count) {
					return r.indexCount < count;
				}
			);
			m_gaps.insert(pos, IndexRange{firstIndex, indexCount});
		}

		/**
		 * @brief Records commands that submits data of vertices
		 * to this vertex buffer on GPU. This is the function that's gonna be
		 * called per-model.
		 * @note Remember: multiple buffers, sharing same memory.
		 * @param indices Array that holds all indices. Must not be empty.
		 * @param commandBuffer Active recording command buffer.
		 * @param stagingBuffer Temporary staging buffer that'll hold temp data.
		 * @param stagingBufferMemoryOffset Offset of buffer in memory.
		 * @param stagingBufferMemory Memory of staging buffer.
		 * @note Must be a part of active command buffer operation.
		 * @warn Make sure to destroy the staging buffers and the memory only after
		 * submittion has been completed.
		 * @note Is not thread-safe. Don't trynna multithread it.
		 * @return IndexRange that holds the first index's position in vertex buffer array, and number of indices.
		 */
		[[nodiscard]] std::tuple<IndexRange, VkBufferCopy> recordIndicesSubmission(
			const std::span<uint32_t> indices,
			VkCommandBuffer commandBuffer,
			VkBuffer stagingBuffer,
			VkDeviceSize stagingBufferMemoryOffset,
			VkDeviceMemory stagingBufferMemory
		)
		{

#ifdef CLZ_ENABLE_CHECKS /*****************************************/
			CLZ_ASSERT(
				!indices.empty(),
				"Attempt to register empty indices array"
			);
			CLZ_ASSERT(
				stagingBufferMemory != VK_NULL_HANDLE,
				"null command buffer passed while registering vertices"
			);
			CLZ_ASSERT(
				commandBuffer != VK_NULL_HANDLE,
				"null command buffer passed"
					" while registering vertices"
			);
			CLZ_ASSERT(
				stagingBuffer != VK_NULL_HANDLE,
				"null buffer passed"
					" while registering vertices"
			);
#endif /***********************************************************/


			const auto& device = r_deviceContext.device;
			const VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

			void* data;
			vkMapMemory(
				device,
				stagingBufferMemory,
				stagingBufferMemoryOffset,
				bufferSize,
				0,
				&data
			);
			memcpy(data, indices.data(), bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			const auto indexCount = static_cast<uint32_t>(indices.size());

			auto retrieveRange = [&](){
				IndexRange outRange;
				if (!m_gaps.empty() && m_gaps.back().indexCount >= indexCount)
				{
					const auto it = std::lower_bound(
						m_gaps.begin(),
						m_gaps.end(),
						indexCount,
						[](const IndexRange& r, const uint32_t count) {
							return r.indexCount < count;
						}
					);

					outRange.firstIndex   = it->firstIndex;
					outRange.indexCount = indexCount;

					it->firstIndex += indexCount;
					it->indexCount -= indexCount;

					if (it->indexCount == 0)
					{
						m_gaps.erase(it);
						return outRange;
					}

					if (it == m_gaps.begin())
						return outRange;

					const auto newPos = std::lower_bound(
						m_gaps.begin(),
						it,
						it->indexCount,
						[](const IndexRange& r, const uint32_t count) {
							return r.indexCount < count;
						}
					);
					if (newPos != it)
					{
						std::reverse(newPos, it);
						std::reverse(newPos, it + 1);
					}

					return outRange;
				}

				outRange.firstIndex   = m_newIndiceStartIndex;
				outRange.indexCount   = indexCount;
				m_newIndiceStartIndex += indexCount;

				return outRange;

			};

			IndexRange indexRange = retrieveRange();

			VkBufferCopy copyRegion{};
			copyRegion.size = bufferSize;
			copyRegion.srcOffset = stagingBufferMemoryOffset;
			copyRegion.dstOffset = indexRange.firstIndex * sizeof(indices[0]);

			return std::make_tuple(indexRange, copyRegion);
		}

		void flushCopies(
			const std::span<VkBufferCopy> copyRegions, 
			VkCommandBuffer commandBuffer, 
			VkBuffer stagingBuffer
		)
		{
			if (copyRegions.empty())
				return;

			vkCmdCopyBuffer(
				commandBuffer, 
				stagingBuffer, 
				m_buffer, 
				static_cast<uint32_t>(copyRegions.size()), 
				copyRegions.data()
			);
		}

		void freeIndices(IndexRange& range)
		{
		#ifdef CLZ_ENABLE_CHECKS
			CLZ_ASSERT(
				range.indexCount != 0,
				"freeing a null/empty vertex range"
			);
		#endif

			uint32_t freedFirstIndex = range.firstIndex;
			uint32_t freedCount = range.indexCount;

			const auto rightIt = std::ranges::find_if(
				m_gaps,
				[rightEnd = freedFirstIndex + freedCount](const IndexRange& r) {
					return r.firstIndex == rightEnd;
				}
			);
			if (rightIt != m_gaps.end())
			{
				freedCount += rightIt->indexCount;
				m_gaps.erase(rightIt);
			}

			const auto leftIt = std::ranges::find_if(
				m_gaps,
				[leftEnd = freedFirstIndex](const IndexRange& r) {
					return r.firstIndex + r.indexCount == leftEnd;
				}
			);
			if (leftIt != m_gaps.end())
			{
				freedFirstIndex = leftIt->firstIndex;
				freedCount += leftIt->indexCount;
				m_gaps.erase(leftIt);
			}

			insertGap(freedFirstIndex, freedCount);
		}
	};

	enum class VertexAttributeType
	{
		POSITION,
		UV,
		NORMAL,
		TANGENT,
	};

	struct StaticMesh
	{
		math::vec3 position;
		math::vec2 uv;
		math::vec3 normal;
		math::vec4 tangent;
	};

	inline VertexBufferInterface<StaticMesh> StaticBufferManager;
	inline IndexBufferInterface StaticIndexManager;

	inline VkVertexInputBindingDescription getStaticVertexBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(StaticMesh);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	inline VkVertexInputAttributeDescription
	getStaticVertexAttributeDescription(const VertexAttributeType type)
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = 0;

		switch (type)
		{
		case VertexAttributeType::POSITION:
			attributeDescription.location = 0;
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(StaticMesh, position);
			break;
		case VertexAttributeType::UV:
			attributeDescription.location = 1;
			attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription.offset = offsetof(StaticMesh, uv);
			break;
		case VertexAttributeType::NORMAL:
			attributeDescription.location = 2;
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(StaticMesh, normal);
			break;
		case VertexAttributeType::TANGENT:
			attributeDescription.location = 3;
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(StaticMesh, tangent);
			break;
		}

		return attributeDescription;
	}
}
