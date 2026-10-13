/**
 * @file arena.cpp
 * @author curl0z
 * @brief Vulkan memory arena allocator
 */

#include "core/assert.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/arena.hpp"
#include "core/logs.hpp"
#include <algorithm>
#include <limits>
#include <vulkan/vulkan_core.h>
#include "core/logs.hpp"

namespace clz::renderer
{
	/// @copydoc allocateVkArenaMemory
	bool allocateVkArenaMemory(const VkArenaDef& arenaDef)
	{
		if (!allocateBufferArena(
				arenaDef.vertexBufferMemSize,
				arenaDef.indexBufferMemSize
		))
		{
			clz::log::error("Unable to allocate vertex buffer memory");
			return false;
		}

		return true;
	}

	/// @copydoc destroyVkArenaMemory
	void destroyVkArenaMemory()
	{
		destroyBufferArena();
	}




	/// @copydoc allocateBufferArena
	bool allocateBufferArena(
		const VkDeviceSize vertexBufferSize,
		const VkDeviceSize indexBufferSize)
	{
		CLZ_ASSERT(
			(vertexBufferSize != 0 || IndexBufferSize != 0),
			"dude why tf u allocating 0 buffer size"
		);
			  
		const auto& device = r_deviceContext.device;

		const VkBufferCreateInfo vBufferCI{
			.sType 		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext 		= nullptr,
			.flags 		= 0,
			.size  		= vertexBufferSize,
			.usage 		= VK_BUFFER_USAGE_TRANSFER_DST_BIT |
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			.sharingMode 	= VK_SHARING_MODE_EXCLUSIVE,

			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices   = nullptr
		};

		if (vkCreateBuffer(
			device,
			&vBufferCI,
			nullptr,
			&VertexBuffer
		) != VK_SUCCESS)
		{
			clz::log::error("Could not create vertex buffer", true);
			return false;
		}

		const VkBufferCreateInfo iBufferCI{
			.sType 		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext 		= nullptr,
			.flags 		= 0,
			.size  		= indexBufferSize,
			.usage 		= VK_BUFFER_USAGE_TRANSFER_DST_BIT |
						VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			.sharingMode 	= VK_SHARING_MODE_EXCLUSIVE,

			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices   = nullptr
		};
		if (vkCreateBuffer(
			device,
			&iBufferCI,
			nullptr,
			&IndexBuffer
		) != VK_SUCCESS)
		{
			clz::log::error("Could not create index buffer", true);
			return false;
		}

		VkMemoryRequirements vMemRequirements;
		VkMemoryRequirements iMemRequirements;
		vkGetBufferMemoryRequirements(device, VertexBuffer, &vMemRequirements);
		vkGetBufferMemoryRequirements(device, IndexBuffer, &iMemRequirements);
		VertexBufferSize = vMemRequirements.size;
		IndexBufferSize  = iMemRequirements.size;
	
		const VkMemoryAllocateInfo vMemAllocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = VertexBufferSize,
			.memoryTypeIndex = findMemoryType(
				vMemRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			),
		};
		const VkMemoryAllocateInfo iMemAllocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = IndexBufferSize,
			.memoryTypeIndex = findMemoryType(
				iMemRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			),
		};


		if (vkAllocateMemory(
				device,
				&vMemAllocInfo,
				nullptr,
				&VertexBufferMemory
		) != VK_SUCCESS)
		{
			clz::log::error("Vulkan could not allocate vertex buffer memory", true);
			return false;
		}
		if (vkAllocateMemory(
				device,
				&iMemAllocInfo,
				nullptr,
				&IndexBufferMemory
		) != VK_SUCCESS)
		{
			clz::log::error("Vulkan could not allocate index buffer memory", true);
			return false;
		}

		
		setHandleName(
			reinterpret_cast<uint64_t>(VertexBufferMemory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			"Vertex buffer memory"
		);
		setHandleName(
			reinterpret_cast<uint64_t>(IndexBufferMemory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			"Index buffer memory"
		);


		vkBindBufferMemory(device, VertexBuffer, VertexBufferMemory, 0);
		vkBindBufferMemory(device, IndexBuffer, IndexBufferMemory, 0);

		VertexBufferNewStartIndex = 0;
		IndexBufferNewStartIndex  = 0;

		return true;
	}

	/// @copydoc destroyBufferArena
	void destroyBufferArena()
	{
		const auto& device = r_deviceContext.device;

		vkDestroyBuffer(device, VertexBuffer, nullptr);
		vkDestroyBuffer(device, IndexBuffer, nullptr);
		vkFreeMemory(device, VertexBufferMemory, nullptr);
		vkFreeMemory(device, IndexBufferMemory, nullptr);
	}

	/// @copydoc allocateVertexBuffer
	bool allocateVertexBuffer(
		const VertexBufferAllocInfo* pAllocInfo,
		clzVertexBuffer* pVertexBuffer
	)
	{
		CLZ_ASSERT(
			pAllocInfo->size != 0,
			"Why are you tring to allocate 0 sized vertex buffer??"
		);

		/// Check if there is any free memory in gaps
		if (!VertexBufferGaps.empty() && 
			VertexBufferGaps.back().size >= pAllocInfo->size)
		{
			auto pOldPos = std::lower_bound(
					VertexBufferGaps.begin(),
					VertexBufferGaps.end(),
					pAllocInfo->size,
					[](const clzVertexBuffer& vBuffer, VkDeviceSize size){
						return vBuffer.size < size;
					}
			);
			/// (it) must not be VertexBufferGaps.end right. RIGHT!!!
			CLZ_ASSERT(
				pOldPos != VertexBufferGaps.end(), 
				"how is it == VertexBufferGaps.end()!!??"
			);

			pVertexBuffer->memoryOffset = pOldPos->memoryOffset;
			pVertexBuffer->size = pAllocInfo->size;
			pOldPos->memoryOffset += pAllocInfo->size;
			pOldPos->size -= pAllocInfo->size;

			/// if new size in pOldPos became 0, remove it.
			if (pOldPos->size == 0)
			{
				VertexBufferGaps.erase(pOldPos);
				/// no need to sort array below. lets bail out.
				return true;
			}


			/// Sort the array again.
			/// Rather than simply using std::sort, use the reverse method.

			/// if this is the first element, simply return, no need to sort.
			if (pOldPos == VertexBufferGaps.begin())
				return true;

			auto pNewPos = std::lower_bound(
					VertexBufferGaps.begin(),
					pOldPos,
					pOldPos->size,
					[](const clzVertexBuffer& vBuffer, VkDeviceSize size){
						return vBuffer.size < size;			
					}
			);

			/// If it turned out to be same position, bail out
			if (pOldPos == pNewPos)
				return true;

			/// finally sort the array
			std::reverse(pNewPos, pOldPos);
			std::reverse(pNewPos, pOldPos + 1);
			return true;
		}

		if ((VertexBufferSize - VertexBufferNewStartIndex) < 
			(pAllocInfo->size)) [[unlikely]]
		{
			clz::log::error("Vertex buffer memory is full, "
					"could not allocate more memory");
			return false;
		}

		pVertexBuffer->memoryOffset = VertexBufferMemoryOffset;
		pVertexBuffer->size = pAllocInfo->size;

		VertexBufferNewStartIndex += pAllocInfo->size;

		return true;
	}

	/// @copydoc allocateIndexBuffer
	bool allocateIndexBuffer(
		const IndexBufferAllocInfo* pAllocInfo,
		clzIndexBuffer* pIndexBuffer
	)
	{
		CLZ_ASSERT(
			pAllocInfo->size != 0,
			"Why are you tring to allocate 0 sized index buffer??"
		);

		/// Check if there is any free memory in gaps
		if (!IndexBufferGaps.empty() && 
			IndexBufferGaps.back().size > pAllocInfo->size)
		{
			auto pOldPos = std::lower_bound(
					IndexBufferGaps.begin(),
					IndexBufferGaps.end(),
					pAllocInfo->size,
					[](const clzIndexBuffer& iBuffer, VkDeviceSize size){
						return iBuffer.size < size;
					}
			);
			/// (it) must not be IndexBufferGaps.end right. RIGHT!!!
			CLZ_ASSERT(
				pOldPos != IndexBufferGaps.end(), 
				"how is it == IndexBufferGaps.end()!!??"
			);

			pIndexBuffer->memoryOffset = pOldPos->memoryOffset;
			pIndexBuffer->size = pAllocInfo->size;
			pOldPos->memoryOffset += pAllocInfo->size;
			pOldPos->size -= pAllocInfo->size;

			/// if new size in pOldPos became 0, remove it.
			if (pOldPos->size == 0)
			{
				IndexBufferGaps.erase(pOldPos);
				/// no need to sort array below. lets bail out.
				return true;
			}


			/// Sort the array again.
			/// Rather than simply using std::sort, use the reverse method.

			/// if this is the first element, simply return, no need to sort.
			if (pOldPos == IndexBufferGaps.begin())
				return true;

			auto pNewPos = std::lower_bound(
					IndexBufferGaps.begin(),
					pOldPos,
					pOldPos->size,
					[](const clzIndexBuffer& iBuffer, VkDeviceSize size){
						return iBuffer.size < size;			
					}
			);

			/// If it turned out to be same position, bail out
			if (pOldPos == pNewPos)
				return true;

			/// finally sort the array
			std::reverse(pNewPos, pOldPos);
			std::reverse(pNewPos, pOldPos + 1);
			return true;
		}

		if ((IndexBufferSize - IndexBufferNewStartIndex) < 
			(pAllocInfo->size)) [[unlikely]]
		{
			clz::log::error("Index buffer memory is full, "
					"could not allocate more memory", false);
			return false;
		}

		pIndexBuffer->memoryOffset = IndexBufferMemoryOffset + IndexBufferNewStartIndex;
		pIndexBuffer->size  = pAllocInfo->size;

		IndexBufferNewStartIndex += pAllocInfo->size;

		return true;
	}

	/// @copydoc freeVertexBuffer
	void freeVertexBuffer(
		clzVertexBuffer* pVertexBuffer
	)
	{
		/// first check if there is any adjacent memory	to the right side
		/// if there is, remove that from and add to the @p handle
		auto rightIt = std::ranges::find_if(
			VertexBufferGaps,
			[rightEnd = (pVertexBuffer->memoryOffset + pVertexBuffer->size)](
				const clzVertexBuffer& vBuffer
			){
				return vBuffer.memoryOffset == rightEnd;	
			}	
		);
		if (rightIt != VertexBufferGaps.end())
		{
			pVertexBuffer->size += rightIt->size;
			VertexBufferGaps.erase(rightIt);
		}

		/// then check for left side. 
		/// If there is, add that to @p handle.
		auto leftIt = std::ranges::find_if(
			VertexBufferGaps,
			[leftEnd = pVertexBuffer->memoryOffset](
				const clzVertexBuffer& vBuffer
			){
				return vBuffer.memoryOffset + vBuffer.size == leftEnd;
			}
		);
		if (leftIt != VertexBufferGaps.end())
		{
			pVertexBuffer->memoryOffset = leftIt->memoryOffset;
			pVertexBuffer->size += leftIt->size;
			VertexBufferGaps.erase(leftIt);
		}

		auto pNewPos = std::lower_bound(
				VertexBufferGaps.begin(),
				VertexBufferGaps.end(),
				pVertexBuffer->size,
				[](const clzVertexBuffer& vBuffer, VkDeviceSize size){
					return vBuffer.size < size;			
				}
		);
		if (pNewPos == VertexBufferGaps.end())
		{
			VertexBufferGaps.push_back(
				clzVertexBuffer{
					.memoryOffset = pVertexBuffer->memoryOffset,
					.size = pVertexBuffer->size
				}
			);
		}
		else
		{
			VertexBufferGaps.insert(
				pNewPos, 
				clzVertexBuffer{
					.memoryOffset = pVertexBuffer->memoryOffset,
					.size = pVertexBuffer->size
				}
			);
		}

		/// at the end, make handle null
		pVertexBuffer->memoryOffset = std::numeric_limits<VkDeviceSize>::max();
		pVertexBuffer->size = 0;
	}
	/// @copydoc freeVertexBuffer
	void freeIndexBuffer(
		clzIndexBuffer* pIndexBuffer
	)
	{
		/// first check if there is any adjacent memory	to the right side
		/// if there is, remove that from and add to the @p handle
		auto rightIt = std::ranges::find_if(
			IndexBufferGaps,
			[rightEnd = (pIndexBuffer->memoryOffset + pIndexBuffer->size)](
				const clzIndexBuffer& iBuffer
			){
				return iBuffer.memoryOffset == rightEnd;	
			}	
		);
		if (rightIt != IndexBufferGaps.end())
		{
			pIndexBuffer->size += rightIt->size;
			IndexBufferGaps.erase(rightIt);
		}

		/// then check for left side. 
		/// If there is, add that to @p handle.
		auto leftIt = std::ranges::find_if(
			IndexBufferGaps,
			[leftEnd = pIndexBuffer->memoryOffset](
				const clzIndexBuffer& iBuffer
			){
				return iBuffer.memoryOffset + iBuffer.size == leftEnd;
			}
		);
		if (leftIt != IndexBufferGaps.end())
		{
			pIndexBuffer->memoryOffset = leftIt->memoryOffset;
			pIndexBuffer->size += leftIt->size;
			IndexBufferGaps.erase(leftIt);
		}

		auto pNewPos = std::lower_bound(
				IndexBufferGaps.begin(),
				IndexBufferGaps.end(),
				pIndexBuffer->size,
				[](const clzIndexBuffer& iBuffer, VkDeviceSize size){
					return iBuffer.size < size;			
				}
		);
		if (pNewPos == IndexBufferGaps.end())
		{
			IndexBufferGaps.push_back(
				clzIndexBuffer{
					.memoryOffset = pIndexBuffer->memoryOffset,
					.size = pIndexBuffer->size
				}
			);
		}
		else
		{
			IndexBufferGaps.insert(
				pNewPos, 
				clzIndexBuffer{
					.memoryOffset = pIndexBuffer->memoryOffset,
					.size = pIndexBuffer->size
				}
			);
		}

		/// at the end, make handle null
		pIndexBuffer->memoryOffset = std::numeric_limits<VkDeviceSize>::max();
		pIndexBuffer->size = 0;
	}

}
