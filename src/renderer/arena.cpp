/**
 * @file arena.cpp
 * @author curl0z
 * @brief Vulkan memory arena allocator
 */

#include "core/assert.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/vk_types.hpp"
#include "renderer/arena.hpp"
#include <algorithm>
#include <limits>
#include "core/logs.hpp"

namespace clz::renderer
{
	/// @copydoc allocateVkArenaMemory
	bool allocateVkArenaMemory(const VkArenaDef& arenaDef)
	{
		if (!allocateBufferArena(
			arenaDef.vertexBufferArenaSize,
			arenaDef.indexBufferArenaSize
		))
		{
			clz::log::error("Unable to allocate vertex buffer arena memory", true);
			return false;
		}

		if (!allocateUBOArenaMemory(
			arenaDef.UBOArenaSize
		))
		{
			clz::log::error("Unable to allocate UBO arena memory", true);
			return false;
		}

		if (!allocateSSBOArenaMemory(
			arenaDef.SSBOArenaSize
		))
		{
			clz::log::error("Unable to allocate SSBO arena memory", true);
			return false;
		}

		if (!ImageMemoryArenas[static_cast<size_t>(
			ImageType::SWAPCHAIN)].initialize(
				"Swapchain",
				arenaDef.SwapchainImagesArenaSize,
				VK_FORMAT_B8G8R8A8_SRGB,
				1,
				VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_TRANSFER_DST_BIT
			)
		)
		{
			clz::log::error("Unable to allocate image memory arena for swapchain", true);
			return false;
		}
		if (!ImageMemoryArenas[static_cast<size_t>(
			ImageType::CANVAS)].initialize(
				"Canvas",
				arenaDef.CanvasImagesArenaSize,
				VK_FORMAT_R16G16B16A16_SFLOAT,
				1,
				VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
					VK_IMAGE_USAGE_SAMPLED_BIT
			)
		)
		{
			clz::log::error("Unable to allocate image memory arena for canvas", true);
			return false;
		}
		if (!ImageMemoryArenas[static_cast<size_t>(
			ImageType::DEPTH)].initialize(
				"Canvas",
				arenaDef.DepthImagesArenaSize,
				VK_FORMAT_D32_SFLOAT,
				1,
				VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_SAMPLED_BIT
			)
		)
		{
			clz::log::error("Unable to allocate image memory arena for depth", true);
			return false;
		}
		if (!ImageMemoryArenas[static_cast<size_t>(
			ImageType::MULTISAMPLED)].initialize(
				"Canvas",
				arenaDef.MSAAImagesArenaSize,
				VK_FORMAT_R16G16B16A16_SFLOAT,
				1,
				VK_SAMPLE_COUNT_8_BIT,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			)
		)
		{
			clz::log::error("Unable to allocate image memory arena for canvas", true);
			return false;
		}
		if (!ImageMemoryArenas[static_cast<size_t>(
			ImageType::TEXTURE)].initialize(
				"Canvas",
				arenaDef.TextureImagesArenaSize,
				VK_FORMAT_R8G8B8A8_SRGB,
				11,
				VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT |
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
					VK_IMAGE_USAGE_SAMPLED_BIT
			)
		)
		{
			clz::log::error("Unable to allocate image memory arena for canvas", true);
			return false;
		}

		return true;
	}

	/// @copydoc destroyVkArenaMemory
	void destroyVkArenaMemory()
	{
		for (auto& arena : ImageMemoryArenas)
			arena.destroy();

		destroySSBOArenaMemory();
		destroyUBOArenaMemory();
		destroyBufferArena();
	}


	static inline uint32_t findMemoryType(
		const uint32_t typeFilter,
		const VkMemoryPropertyFlags properties
	)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(
			clz::renderer::r_deviceContext.physicalDevice,
			&memProperties
		);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
			    (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		clz::log::error("failed to find suitable memory type!");
		clz::CLZ_ASSERT(false, "could not find suitable memory type!");
		return UINT32_MAX;
	}


	/// @copydoc allocateBufferArena
	bool allocateBufferArena(
		const VkDeviceSize vertexBufferArenaSize,
		const VkDeviceSize indexBufferArenaSize)
	{
		CLZ_ASSERT(
			(vertexBufferArenaSize != 0 && indexBufferArenaSize != 0),
			"Attempt to allocate arena buffer of size 0"
		);
			  
		const auto& device = r_deviceContext.device;

		const VkBufferCreateInfo vBufferCI{
			.sType 		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext 		= nullptr,
			.flags 		= 0,
			.size  		= vertexBufferArenaSize,
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
			.size  		= indexBufferArenaSize,
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
		VertexArenaTotalSize = vMemRequirements.size;
		IndexArenaTotalSize   = iMemRequirements.size;
	
		const VkMemoryAllocateInfo vMemAllocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = VertexArenaTotalSize,
			.memoryTypeIndex = findMemoryType(
				vMemRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			),
		};
		const VkMemoryAllocateInfo iMemAllocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = IndexArenaTotalSize,
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

		if ((VertexArenaTotalSize - VertexBufferNewStartIndex) < 
			(pAllocInfo->size)) [[unlikely]]
		{
			clz::log::error("Vertex buffer memory is full, "
					"could not allocate more memory");
			return false;
		}

		pVertexBuffer->memoryOffset = VertexBufferNewStartIndex;
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

		if ((IndexArenaTotalSize - IndexBufferNewStartIndex) < 
			(pAllocInfo->size)) [[unlikely]]
		{
			clz::log::error("Index buffer memory is full, "
					"could not allocate more memory", false);
			return false;
		}

		pIndexBuffer->memoryOffset = IndexBufferNewStartIndex;
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


	/// @copydoc allocateUBOMemory
	bool allocateUBOArenaMemory(
		const VkDeviceSize uboArenaSize
	)
	{
		CLZ_ASSERT(
			uboArenaSize != 0, 
			"UBO Arena size is passed to be 0. But it should not be 0 at all"
		);

		const VkDevice& device = r_deviceContext.device;

		/// make sure memory size is divisible by min ubo offset alignment
		const auto MinUBOAlignmentSize = [&](){
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(r_deviceContext.physicalDevice, &properties);
			return properties.limits.minUniformBufferOffsetAlignment;
		}();

		auto TotalMemToAllocate = [&](){
			return (MinUBOAlignmentSize + uboArenaSize - 1) & 
				~(MinUBOAlignmentSize - 1);
		}();


		const VkBufferCreateInfo bufferCI{
			.sType 		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext 		= nullptr,
			.flags 		= 0,
			.size  		= TotalMemToAllocate,
			.usage 		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			.sharingMode 	= VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices   = nullptr
		};
		const VkDeviceBufferMemoryRequirements bufferMemReq{
			.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
			.pNext = nullptr,
			.pCreateInfo = &bufferCI
		};

		VkMemoryRequirements2 memRequirements{
			.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
			.pNext = nullptr
		};
		vkGetDeviceBufferMemoryRequirements(
			device,
			&bufferMemReq,
			&memRequirements
		);

		constexpr VkMemoryPropertyFlags hostFlags =
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		const VkMemoryPropertyFlags candidates[] = {
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | hostFlags,
			hostFlags
		};

		bool allocated = false;
		for (const VkMemoryPropertyFlags flags : candidates)
		{
			const uint32_t type = findMemoryType(
				memRequirements.memoryRequirements.memoryTypeBits,
				flags
			);
			if (type == UINT32_MAX)
				continue;

			const VkMemoryAllocateInfo memAllocInfo{
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = nullptr,
				.allocationSize = TotalMemToAllocate,
				.memoryTypeIndex = type
			};
			if (vkAllocateMemory(
				device,
				&memAllocInfo,
				nullptr,
				&SSBOArenaMemory
			) == VK_SUCCESS)
			{
				allocated = true;
				break;
			}
		}

		if (!allocated)
		{
			clz::log::error("Vulkan could not allocate ssbo arena memory", true);
			return false;
		}

		if (vkMapMemory(
			device,
			UBOArenaMemory,
			0,
			VK_WHOLE_SIZE,
			0,
			&UBOArenaMapped
		) != VK_SUCCESS)
		{
			log::error(
				"Could not map ssbo arena memory",
				true
			);
			return false;
		}

		setHandleName(
			reinterpret_cast<uint64_t>(UBOArenaMemory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			"UBO arena memory"
		);

		UBOArenaTotalSize = TotalMemToAllocate;
		UBOMemoryNewStartIndex = 0;

		return true;
	}

	/// @copydoc destroyUBOArenaMemory
	void destroyUBOArenaMemory()
	{
		vkUnmapMemory(r_deviceContext.device, UBOArenaMemory);
		vkFreeMemory(
			r_deviceContext.device,
			UBOArenaMemory,
			nullptr
		);

		UBOArenaMemory = VK_NULL_HANDLE;
		UBOArenaMapped = nullptr;
		UBOArenaGaps.clear();
		UBOArenaTotalSize = 0;
		UBOMemoryNewStartIndex = 0;
	}

	/// @copydoc carveUBOMemory
	bool carveUBOMemory(
		const UBOMemoryAllocInfo& rAllocInfo,
		clzUBOMemory& rUBOMemoryHandle
	)
	{
#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			UBOArenaMemory != VK_NULL_HANDLE,
			"UBO Arena memory is not initialized, "
			"but attempt was made to carve memory from it."
		);
		CLZ_ASSERT(
			rAllocInfo.size != 0 && rAllocInfo.alignment != 0,
			"Can't allocate a ubo memory of size 0 or alignment 0"
		);
		CLZ_ASSERT(
			!rUBOMemoryHandle.isNull(),
			"Attempt to carve ubo memory for a non null handle"
		);
#endif

		auto getNearestOffset = [alignment = rAllocInfo.alignment](
						VkDeviceSize prvOffset
					){
						return (alignment + prvOffset - 1) & 
							~(alignment - 1);
					};

		VkDeviceSize memToAllocate = getNearestOffset(rAllocInfo.size);

		if (!UBOArenaGaps.empty() &&
			UBOArenaGaps.back().getSize() >= memToAllocate)
		{
			auto it = std::lower_bound(
					UBOArenaGaps.begin(),
					UBOArenaGaps.end(),
					memToAllocate,
					[](const clzUBOMemory& uboMemory, VkDeviceSize size){
						return uboMemory.getSize() < size;
					}
			);

			for (auto cand = it; cand != UBOArenaGaps.end(); ++cand)
			{
				const auto nextOffset = getNearestOffset(cand->getMemoryOffset());

				/// if after aligning, memory is less, continue to next iteration
				if (cand->getEnd() < (nextOffset + memToAllocate))
					continue;

				rUBOMemoryHandle.set(nextOffset, memToAllocate);

				/// handle left and right gap.
				clzUBOMemory leftGap;
				clzUBOMemory rightGap;

				/// construct the gaps
				if (nextOffset - cand->getMemoryOffset() != 0)
				{
					leftGap.set(
						cand->getMemoryOffset(),
						nextOffset - cand->getMemoryOffset()
					);
				}
				if (cand->getEnd() > rUBOMemoryHandle.getEnd())
				{
					rightGap.set(
						rUBOMemoryHandle.getEnd(),
						cand->getEnd() - rUBOMemoryHandle.getEnd()
					);
				}

				/// erase the current gap
				UBOArenaGaps.erase(cand);

				/// push back the new gaps
				if (!leftGap.isNull())
				{
					auto leftIt = std::lower_bound(
						UBOArenaGaps.begin(),
						UBOArenaGaps.end(),
						leftGap.getSize(),
						[](const clzUBOMemory& uboMemory, VkDeviceSize size){
							return uboMemory.getSize() < size;
						}
					);
					UBOArenaGaps.insert(leftIt, leftGap);
				}
				if (!rightGap.isNull())
				{
					auto rightIt = std::lower_bound(
						UBOArenaGaps.begin(),
						UBOArenaGaps.end(),
						rightGap.getSize(),
						[](const clzUBOMemory& uboMemory, VkDeviceSize size){
							return uboMemory.getSize() < size;
						}
					);
					UBOArenaGaps.insert(rightIt, rightGap);
				}

				return true;
			}
		}



		const auto newStartIndex = (rAllocInfo.alignment + UBOMemoryNewStartIndex - 1) & 
						~(rAllocInfo.alignment - 1);
		if ((UBOArenaTotalSize) < (newStartIndex + memToAllocate)) [[unlikely]]
		{
			clz::log::error("ubo memory is full, "
					"could not allocate more memory");
			return false;
		}

		rUBOMemoryHandle.set(newStartIndex, memToAllocate);

		if (UBOMemoryNewStartIndex != newStartIndex)
		{
			clzUBOMemory gap;
			gap.set(UBOMemoryNewStartIndex, newStartIndex - UBOMemoryNewStartIndex);
			auto it = std::lower_bound(
					UBOArenaGaps.begin(),
					UBOArenaGaps.end(),
					gap.getSize(),
					[](const clzUBOMemory& uboMemory, VkDeviceSize size){
						return uboMemory.getSize() < size;
					}
			);
			UBOArenaGaps.insert(it, gap);
		}

		UBOMemoryNewStartIndex = newStartIndex + memToAllocate;

		return true;
	}

	/// @copydoc freeUBOMemory
	void freeUBOMemory(
		clzUBOMemory& rUboMemoryHandle
	)
	{
#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			UBOArenaMemory != VK_NULL_HANDLE,
			"UBO Arena memory is not initialized, "
			"but attempt was made to free memory from it."
		);
		CLZ_ASSERT(
			!rUboMemoryHandle.isNull(),
			"Attempt to destroy a null UBO memory. "
			"Can't destroy a null handle"
		);
#endif

		/// first check if there is any right adjacent memory.
		auto rightIt = std::ranges::find_if(
			UBOArenaGaps,
			[rightEnd = rUboMemoryHandle.getEnd()](
				const clzUBOMemory& uboMemory
			){
				return rightEnd == uboMemory.getMemoryOffset();
			}
		);
		if (rightIt != UBOArenaGaps.end())
		{
			rUboMemoryHandle.setSize(
				rUboMemoryHandle.getSize() + rightIt->getSize()
			);
			UBOArenaGaps.erase(rightIt);
		}

		/// then check left adjacency.
		auto leftIt = std::ranges::find_if(
			UBOArenaGaps,
			[leftEnd = rUboMemoryHandle.getMemoryOffset()](
				const clzUBOMemory& uboMemory
			) {
				return uboMemory.getEnd() == leftEnd;
			}
		);
		if (leftIt != UBOArenaGaps.end())
		{
			rUboMemoryHandle.set(
				leftIt->getMemoryOffset(),
				rUboMemoryHandle.getSize() + leftIt->getSize()
			);
			UBOArenaGaps.erase(leftIt);
		}

		auto newPos = std::lower_bound(
			UBOArenaGaps.begin(),
			UBOArenaGaps.end(),
			rUboMemoryHandle.getSize(),
			[](const clzUBOMemory& uboMemory, const VkDeviceSize size) {
				return uboMemory.getSize() < size;
			}
		);
		UBOArenaGaps.insert(newPos, rUboMemoryHandle);

		rUboMemoryHandle.makeNull();
	}
	


	/// @copydoc allocateSSBOArenaMemory
	bool allocateSSBOArenaMemory(const VkDeviceSize ssboArenaSize)
	{
		CLZ_ASSERT(ssboArenaSize != 0, "SSBO arena size is passed to be 0");

		const VkDevice& device = r_deviceContext.device;
		/// make sure memory size is divisible by min ubo offset alignment
		const auto MinSSBOAlignmentSize = [&]() {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(r_deviceContext.physicalDevice, &properties);
			return properties.limits.minUniformBufferOffsetAlignment;
		}();

		auto TotalMemToAllocate = [&]() {
			return (MinSSBOAlignmentSize + ssboArenaSize - 1) &
			       ~(MinSSBOAlignmentSize - 1);
		}();

		/// Usage flags here must match the real SSBO buffers,
		/// because memoryTypeBits depends on them.
		const VkBufferCreateInfo bufferCI{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.size = TotalMemToAllocate,
			.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr
		};
		const VkDeviceBufferMemoryRequirements bufferMemReq{
			.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
			.pNext = nullptr,
			.pCreateInfo = &bufferCI
		};

		VkMemoryRequirements2 memRequirements{
			.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2
		};
		vkGetDeviceBufferMemoryRequirements(
			device,
			&bufferMemReq,
			&memRequirements
		);

		constexpr VkMemoryPropertyFlags hostFlags =
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		const VkMemoryPropertyFlags candidates[] = {
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | hostFlags,
			hostFlags
		};

		bool allocated = false;
		for (const VkMemoryPropertyFlags flags : candidates)
		{
			const uint32_t type = findMemoryType(
				memRequirements.memoryRequirements.memoryTypeBits,
				flags
			);
			if (type == UINT32_MAX)
				continue;

			const VkMemoryAllocateInfo memAllocInfo{
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = nullptr,
				.allocationSize = TotalMemToAllocate,
				.memoryTypeIndex = type
			};
			if (vkAllocateMemory(
				    device,
				    &memAllocInfo,
				    nullptr,
				    &SSBOArenaMemory
			    ) == VK_SUCCESS)
			{
				allocated = true;
				break;
			}
		}

		if (!allocated)
		{
			clz::log::error(
				"Vulkan could not allocate ssbo arena memory",
				true
			);
			return false;
		}

		if (vkMapMemory(
			    device,
			    SSBOArenaMemory,
			    0,
			    VK_WHOLE_SIZE,
			    0,
			    &SSBOArenaMapped
		) != VK_SUCCESS)
		{
			clz::log::error("Could not map ssbo arena memory", true);
			vkFreeMemory(device, SSBOArenaMemory, nullptr);
			SSBOArenaMemory = VK_NULL_HANDLE;
			return false;
		}

		setHandleName(
			reinterpret_cast<uint64_t>(SSBOArenaMemory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			"SSBO arena memory"
		);

		SSBOArenaGaps.clear();
		SSBOArenaTotalSize = ssboArenaSize;
		SSBOMemoryNewStartIndex = 0;

		return true;
	}

	/// @copydoc destroySSBOArenaMemory
	void destroySSBOArenaMemory()
	{
		const VkDevice& device = r_deviceContext.device;

		if (SSBOArenaMapped != nullptr)
			vkUnmapMemory(device, SSBOArenaMemory);
		vkFreeMemory(device, SSBOArenaMemory, nullptr);

		SSBOArenaMemory = VK_NULL_HANDLE;
		SSBOArenaMapped = nullptr;
		SSBOArenaTotalSize = 0;
		SSBOMemoryNewStartIndex = 0;
		SSBOArenaGaps.clear();
	}

	/// Inserts a gap keeping SSBOArenaGaps sorted by size. Skips empty gaps.
	static inline void insertSSBOGap(const VkDeviceSize offset, const VkDeviceSize size)
	{
		if (size == 0)
			return;

		clzSSBOMemory gap;
		gap.set(offset, size);

		const auto pos = std::lower_bound(
			SSBOArenaGaps.begin(),
			SSBOArenaGaps.end(),
			size,
			[](const clzSSBOMemory& m, const VkDeviceSize s) {
				return m.getSize() < s;
			}
		);
		SSBOArenaGaps.insert(pos, gap);
	}

	/// @copydoc carveSSBOMemory
	bool carveSSBOMemory(
		const SSBOMemoryAllocInfo& rAllocInfo,
		clzSSBOMemory& rSSBOMemoryHandle
	)
	{
#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			SSBOArenaMemory != VK_NULL_HANDLE,
			"SSBO Arena memory is not initialized, "
			"but attempt was made to carve memory from it."
		);
		CLZ_ASSERT(
			rAllocInfo.size != 0 && rAllocInfo.alignment != 0,
			"ssbo size must be non-zero and alignment a power of two"
		);
		CLZ_ASSERT(
			rSSBOMemoryHandle.isNull(),
			"ssbo handle already owns memory, free it first"
		);
#endif

		const auto getNearestOffset   = [alignment = rAllocInfo.alignment](
							const VkDeviceSize v
						)
						{
							return (v + alignment - 1) &
								~(alignment - 1);
						};

		const VkDeviceSize memToAllocate = getNearestOffset(rAllocInfo.size);

		if (!SSBOArenaGaps.empty() &&
			SSBOArenaGaps.back().getSize() >= memToAllocate)
		{
			const auto it = std::lower_bound(
				SSBOArenaGaps.begin(),
				SSBOArenaGaps.end(),
				memToAllocate,
				[](const clzSSBOMemory& ssboMemory, const VkDeviceSize size) {
					return ssboMemory.getSize() < size;
				}
			);

			for (auto cand = it; cand != SSBOArenaGaps.end(); ++cand)
			{
				const auto nextOffset =
					getNearestOffset(cand->getMemoryOffset());

				/// if after aligning memory is less, continue to next iteration
				if (cand->getEnd() < (nextOffset + memToAllocate))
					continue;

				rSSBOMemoryHandle.set(nextOffset, memToAllocate);

				/// handle left and right gap.
				clzSSBOMemory leftGap;
				clzSSBOMemory rightGap;

				/// construct the gaps
				if (nextOffset - cand->getMemoryOffset() != 0)
				{
					leftGap.set(
						cand->getMemoryOffset(),
						nextOffset - cand->getMemoryOffset()
					);
				}
				if (cand->getEnd() > rSSBOMemoryHandle.getEnd())
				{
					rightGap.set(
						rSSBOMemoryHandle.getEnd(),
						cand->getEnd() - rSSBOMemoryHandle.getEnd()
					);
				}

				/// erase the current gap
				SSBOArenaGaps.erase(cand);

				/// insert the new gaps, sorted by their own size
				if (!leftGap.isNull())
				{
					auto leftIt = std::lower_bound(
						SSBOArenaGaps.begin(),
						SSBOArenaGaps.end(),
						leftGap.getSize(),
						[](const clzSSBOMemory& ssboMemory, VkDeviceSize size) {
							return ssboMemory.getSize() < size;
						}
					);
					SSBOArenaGaps.insert(leftIt, leftGap);
				}
				if (!rightGap.isNull())
				{
					auto rightIt = std::lower_bound(
						SSBOArenaGaps.begin(),
						SSBOArenaGaps.end(),
						rightGap.getSize(),
						[](const clzSSBOMemory& ssboMemory, VkDeviceSize size) {
							return ssboMemory.getSize() < size;
						}
					);
					SSBOArenaGaps.insert(rightIt, rightGap);
				}

				return true;
			}
		}

		/// 2. Bump allocate from fresh memory.
		const VkDeviceSize newOffset = getNearestOffset(SSBOMemoryNewStartIndex);
		if (newOffset + memToAllocate > SSBOArenaTotalSize) [[unlikely]]
		{
			clz::log::error(
				"ssbo memory is full, "
				"could not allocate more memory"
			);
			return false;
		}

		insertSSBOGap(SSBOMemoryNewStartIndex, newOffset - SSBOMemoryNewStartIndex);

		rSSBOMemoryHandle.set(newOffset, memToAllocate);
		SSBOMemoryNewStartIndex = newOffset + memToAllocate;

		return true;
	}

	/// @copydoc freeSSBOMemory
	void freeSSBOMemory(clzSSBOMemory& rSSBOMemoryHandle)
	{
#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			SSBOArenaMemory != VK_NULL_HANDLE,
			"SSBO Arena memory is not initialized, "
			"but attempt was made to free memory from it."
		);
		CLZ_ASSERT(
			!rSSBOMemoryHandle.isNull(),
			"freeing a null ssbo handle"
		);
#endif

		/// Merge with a gap that starts exactly where this block ends.
		const auto rightIt = std::ranges::find_if(
			SSBOArenaGaps,
			[rightEnd = rSSBOMemoryHandle.getEnd()](
				const clzSSBOMemory& ssboMemoryGaps
			){
				return ssboMemoryGaps.getMemoryOffset() == rightEnd;
			}
		);
		if (rightIt != SSBOArenaGaps.end())
		{
			rSSBOMemoryHandle.setSize(
				rSSBOMemoryHandle.getSize() + rightIt->getSize()
			);
			SSBOArenaGaps.erase(rightIt);
		}

		/// Merge with a gap that ends exactly where this block starts.
		const auto leftIt = std::ranges::find_if(
			SSBOArenaGaps,
			[leftEnd = rSSBOMemoryHandle.getMemoryOffset()](
				const clzSSBOMemory& ssboMemoryGap
			){
				return ssboMemoryGap.getEnd() == leftEnd;
			}
		);
		if (leftIt != SSBOArenaGaps.end())
		{
			rSSBOMemoryHandle.set(
				leftIt->getMemoryOffset(),
				rSSBOMemoryHandle.getSize() + leftIt->getSize()
			);
			SSBOArenaGaps.erase(leftIt);
		}

		insertSSBOGap(
			rSSBOMemoryHandle.getMemoryOffset(),
			rSSBOMemoryHandle.getSize()
		);

		rSSBOMemoryHandle.makeNull();
	}


	/// @copydoc ImageMemoryArena::initialize
	bool ImageMemoryArena::initialize(
		const std::string& arenaName,
		VkDeviceSize arenaSize,
		VkFormat format,
		uint8_t mipLevels,
		VkSampleCountFlagBits sampleBits,
		VkImageTiling tiling,
		VkImageUsageFlags usageFlags
	)
	{
		CLZ_ASSERT(
			arenaSize != 0,
			"Attempt to create image arena with memory size is 0"
			". Memory must be >= 0"
		);
	
		const auto& device = r_deviceContext.device;

		const VkImageCreateInfo imageCI{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = {1920, 1080, 1},
			.mipLevels = mipLevels,
			.arrayLayers = 1,
			.samples = sampleBits,
			.tiling = tiling,
			.usage = usageFlags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		const VkDeviceImageMemoryRequirements imageMemReq{
			.sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS,
			.pNext = nullptr,
			.pCreateInfo = &imageCI,
			.planeAspect = VK_IMAGE_ASPECT_NONE
		};

		VkMemoryRequirements2 memRequirements{
			.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2
		};
		vkGetDeviceImageMemoryRequirements(device, &imageMemReq, &memRequirements);

		const uint32_t type =
			findMemoryType(
				memRequirements.memoryRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
		if (type == UINT32_MAX)
		{
			clz::log::error(
				"No device local memory type for arena: " + arenaName,
				true
			);
			return false;
		}

		const VkMemoryAllocateInfo memAllocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = arenaSize,
			.memoryTypeIndex = type
		};
		if (vkAllocateMemory(
			    device,
			    &memAllocInfo,
			    nullptr,
			    &m_arenaMemory
		    ) != VK_SUCCESS)
		{
			clz::log::error(
				"Vulkan could not allocate image arena memory for: " + arenaName,
				true
			);
			return false;
		}

		setHandleName(
			reinterpret_cast<uint64_t>(m_arenaMemory),
			VK_OBJECT_TYPE_DEVICE_MEMORY,
			std::string(arenaName + " memory arena").c_str()
		);

		m_arenaName = arenaName;
		m_totalMemory = arenaSize;
		m_newStartIndex = 0;
		m_gaps.clear();

		return true;
	}

	/// @copydoc ImageMemoryArena::destroy
	void ImageMemoryArena::destroy()
	{
		vkFreeMemory(r_deviceContext.device, m_arenaMemory, nullptr);
		m_totalMemory = 0;
		m_newStartIndex = 0;
		m_gaps.clear();
	}

	/// @copydoc ImageMemoryArena::insertGap
	void ImageMemoryArena::insertGap(const clzImageMemory& rGap)
	{
		const auto pos = std::lower_bound(
			m_gaps.begin(),
			m_gaps.end(),
			rGap.getSize(),
			[](const clzImageMemory& imageMemory, const VkDeviceSize size) {
				return imageMemory.getSize() < size;
			}
		);
		m_gaps.insert(pos, rGap);
	}

	/// @copydoc ImageMemoryArena::carveMemory
	bool ImageMemoryArena::carveMemory(
		const ImageMemoryAllocInfo& rAllocInfo,
		clzImageMemory& rImageMemory
	)
	{
#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			m_arenaMemory != VK_NULL_HANDLE,
			"image memory arena has'nt been initialized yet, "
			"but attempt was made to carve memory from it."
		);
		CLZ_ASSERT(
			rAllocInfo.size != 0 && rAllocInfo.alignment != 0,
			"while carving texture memory, size and alignment must be non-zero"
		);
		CLZ_ASSERT(
			rImageMemory.isNull(),
			"attempt to allocate memory to a non-null image memory handle"
		);
#endif

		auto getNearestOffset = [alignment = rAllocInfo.alignment](
						VkDeviceSize prvOffset
					)
					{
						return (alignment + prvOffset - 1) &
							~(alignment - 1);
					};

		const VkDeviceSize memToAllocate = rAllocInfo.size;

		if (!m_gaps.empty() &&
			m_gaps.back().getSize() >= memToAllocate)
		{
			auto it = std::lower_bound(
				m_gaps.begin(),
				m_gaps.end(),
				memToAllocate,
				[](const clzImageMemory & imageMemory, VkDeviceSize size) {
					return imageMemory.getSize() < size;
				}
			);

			for (auto cand = it; cand != m_gaps.end(); ++cand)
			{
				const auto nextOffset =
					getNearestOffset(cand->getMemoryOffset());

				/// if after aligning, memory is less,
				/// continue to next iteration
				if (cand->getEnd() < (nextOffset + memToAllocate))
					continue;


				rImageMemory.set(nextOffset, memToAllocate);

				/// handle left and right gap.
				clzImageMemory leftGap;
				clzImageMemory rightGap;

				if (nextOffset - cand->getMemoryOffset() != 0)
				{
					leftGap.set(
						cand->getMemoryOffset(),
						nextOffset - cand->getMemoryOffset()
					);
				}
				if (cand->getEnd() > rImageMemory.getEnd())
				{
					rightGap.set(
						rImageMemory.getEnd(),
						cand->getEnd() - rImageMemory.getEnd()
					);
				}

				/// erase the current gap
				m_gaps.erase(cand);

				if (!leftGap.isNull())
					insertGap(leftGap);
				if (!rightGap.isNull())
					insertGap(rightGap);

				return true;
			}
		}

		const auto newStartIndex =
			getNearestOffset(m_newStartIndex);
		if (m_totalMemory < (m_newStartIndex + memToAllocate))
		{
			clz::log::error(
				m_arenaName + "arena is full, "
				"could not allocate more memory"
			);
			return false;
		}

		rImageMemory.set(newStartIndex, memToAllocate);

		if (m_newStartIndex != newStartIndex)
		{
			clzImageMemory gap;
			gap.set(
				m_newStartIndex,
				newStartIndex - m_newStartIndex
			);
			insertGap(gap);
		}

		m_newStartIndex = newStartIndex + memToAllocate;

		return true;

	}

	/// @copydoc ImageMemoryArena::freeMemory
	void ImageMemoryArena::freeMemory(clzImageMemory& rImageMemory)
	{
#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			m_arenaMemory != VK_NULL_HANDLE,
			"image memory arena has'nt been initialized yet, "
			"but attempt was made to free memory from it."
		);
		CLZ_ASSERT(
			!rImageMemory.isNull(),
			"Attempt to free a null image memory handle"
		);
#endif

		/// first check if there is any right adjacent memory.
		auto rightIt = std::ranges::find_if(
			m_gaps,
			[rightEnd = rImageMemory.getEnd()](
				const clzImageMemory& imageMemory
			)
			{
				return rightEnd == imageMemory.getMemoryOffset();
			}
		);
		if (rightIt != m_gaps.end())
		{
			rImageMemory.setSize(rImageMemory.getSize() + rightIt->getSize());
			m_gaps.erase(rightIt);
		}

		/// then check left adjacency.
		auto leftIt = std::ranges::find_if(
			m_gaps,
			[leftEnd = rImageMemory.getMemoryOffset()](
				const clzImageMemory& imageMemory
			)
			{
				return imageMemory.getEnd() == leftEnd;
			}
		);
		if (leftIt != m_gaps.end())
		{
			rImageMemory.set(
				leftIt->getMemoryOffset(),
				rImageMemory.getSize() + leftIt->getSize()
			);
			m_gaps.erase(leftIt);
		}

		insertGap(rImageMemory);
		rImageMemory.makeNull();
	}

}
