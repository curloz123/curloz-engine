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

		const VkDeviceBufferMemoryRequirements vBufferMemReq{
			.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
			.pNext = nullptr,
			.pCreateInfo = &vBufferCI
		};
		VkMemoryRequirements2 vMemRequirements{
			.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
			.pNext = nullptr
		};
		vkGetDeviceBufferMemoryRequirements(
			device,
			&vBufferMemReq,
			&vMemRequirements
		);

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

		const VkDeviceBufferMemoryRequirements iBufferMemReq{
			.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
			.pNext = nullptr,
			.pCreateInfo = &iBufferCI
		};
		VkMemoryRequirements2 iMemRequirements{
			.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
			.pNext = nullptr
		};
		vkGetDeviceBufferMemoryRequirements(
			device,
			&iBufferMemReq,
			&iMemRequirements
		);

		VertexArenaTotalSize = vMemRequirements.memoryRequirements.size;
		IndexArenaTotalSize   = iMemRequirements.memoryRequirements.size;

		const VkMemoryAllocateInfo vMemAllocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = VertexArenaTotalSize,
			.memoryTypeIndex = findMemoryType(
				vMemRequirements.memoryRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			),
		};
		const VkMemoryAllocateInfo iMemAllocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = IndexArenaTotalSize,
			.memoryTypeIndex = findMemoryType(
				iMemRequirements.memoryRequirements.memoryTypeBits,
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


		VertexBufferNewStartIndex = 0;
		IndexBufferNewStartIndex  = 0;

		return true;
	}

	/// @copydoc destroyBufferArena
	void destroyBufferArena()
	{
		const auto& device = r_deviceContext.device;

		vkFreeMemory(device, VertexBufferMemory, nullptr);
		vkFreeMemory(device, IndexBufferMemory, nullptr);
	}



	/// Inserts a gap keeping VertexArenaGaps sorted by size. Skips empty gaps.
	static inline void insertVertexGap(
		const VkDeviceSize offset, 
		const VkDeviceSize size
	)
	{
		if (size == 0)
			return;

		if (offset + size == VertexBufferNewStartIndex)
		{
			VertexBufferNewStartIndex = offset;
			return;
		}

		clzVertexBufferMemory gap;
		gap.set(offset, size);

		const auto pos = std::lower_bound(
			VertexArenaGaps.begin(),
			VertexArenaGaps.end(),
			size,
			[](const clzVertexBufferMemory& m, const VkDeviceSize s) {
				return m.getSize() < s;
			}
		);
		VertexArenaGaps.insert(pos, gap);
	}

	/// Inserts a gap keeping IndexArenaGaps sorted by size. Skips empty gaps.
	static inline void insertIndexGap(
		const VkDeviceSize offset,
		const VkDeviceSize size
	)
	{
		if (size == 0)
			return;

		if (offset + size == IndexBufferNewStartIndex)
		{
			IndexBufferNewStartIndex = offset;
			return;
		}

		clzIndexBufferMemory gap;
		gap.set(offset, size);

		const auto pos = std::lower_bound(
			IndexArenaGaps.begin(),
			IndexArenaGaps.end(),
			size,
			[](const clzIndexBufferMemory& m, const VkDeviceSize s) {
				return m.getSize() < s;
			}
		);
		IndexArenaGaps.insert(pos, gap);
	}

	/// @copydoc allocateVertexBufferMemory
	bool allocateVertexBufferMemory(
		const VertexBufferMemoryAllocationInfo& rAllocInfo,
		clzVertexBufferMemory& rBufferMemory
	)
	{
	#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			VertexBufferMemory != VK_NULL_HANDLE,
			"Vertex buffer arena memory is not initialized, "
			"but attempt was made to carve memory from it."
		);
		CLZ_ASSERT(
			rAllocInfo.size != 0 && rAllocInfo.alignment != 0,
			"vertex buffer size must be non-zero and alignment a power of two"
		);
		CLZ_ASSERT(
			rBufferMemory.isNull(),
			"vertex buffer handle already owns memory, free it first"
		);
	#endif

		const auto getNearestOffset = [alignment = rAllocInfo.alignment](
							const VkDeviceSize v
						)
						{
							return (v + alignment - 1) &
								~(alignment - 1);
						};

		const VkDeviceSize memToAllocate = getNearestOffset(rAllocInfo.size);

		if (!VertexArenaGaps.empty() &&
			VertexArenaGaps.back().getSize() >= memToAllocate)
		{
			const auto it = std::lower_bound(
				VertexArenaGaps.begin(),
				VertexArenaGaps.end(),
				memToAllocate,
				[](const clzVertexBufferMemory& vBufferMemory, const VkDeviceSize size) {
					return vBufferMemory.getSize() < size;
				}
			);

			for (auto cand = it; cand != VertexArenaGaps.end(); ++cand)
			{
				const auto nextOffset =
					getNearestOffset(cand->getMemoryOffset());

				/// if after aligning memory is less, continue to next iteration
				if (cand->getEnd() < (nextOffset + memToAllocate))
					continue;

				rBufferMemory.set(nextOffset, memToAllocate);

				/// handle left and right gap.
				clzVertexBufferMemory leftGap;
				clzVertexBufferMemory rightGap;

				if (nextOffset - cand->getMemoryOffset() != 0)
				{
					leftGap.set(
						cand->getMemoryOffset(),
						nextOffset - cand->getMemoryOffset()
					);
				}
				if (cand->getEnd() > rBufferMemory.getEnd())
				{
					rightGap.set(
						rBufferMemory.getEnd(),
						cand->getEnd() - rBufferMemory.getEnd()
					);
				}

				/// erase the current gap
				VertexArenaGaps.erase(cand);

				/// insert the new gaps, sorted by their own size
				if (!leftGap.isNull())
					insertVertexGap(leftGap.getMemoryOffset(), leftGap.getSize());
				if (!rightGap.isNull())
					insertVertexGap(rightGap.getMemoryOffset(), rightGap.getSize());

				return true;
			}
		}

		/// 2. Bump allocate from fresh memory.
		const VkDeviceSize newOffset = getNearestOffset(VertexBufferNewStartIndex);
		if (newOffset + memToAllocate > VertexArenaTotalSize) [[unlikely]]
		{
			clz::log::error(
				"vertex buffer memory is full, "
				"could not allocate more memory"
			);
			return false;
		}

		insertVertexGap(VertexBufferNewStartIndex, newOffset - VertexBufferNewStartIndex);

		rBufferMemory.set(newOffset, memToAllocate);
		VertexBufferNewStartIndex = newOffset + memToAllocate;

		return true;
	}

	/// @copydoc allocateIndexBufferMemory
	bool allocateIndexBufferMemory(
		const IndexBufferMemoryAllocationInfo& rAllocInfo,
		clzIndexBufferMemory& rBufferMemory
	)
	{
	#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			IndexBufferMemory != VK_NULL_HANDLE,
			"Index buffer arena memory is not initialized, "
			"but attempt was made to carve memory from it."
		);
		CLZ_ASSERT(
			rAllocInfo.size != 0 && rAllocInfo.alignment != 0,
			"index buffer size must be non-zero and alignment a power of two"
		);
		CLZ_ASSERT(
			rBufferMemory.isNull(),
			"index buffer handle already owns memory, free it first"
		);
	#endif

		const auto getNearestOffset = [alignment = rAllocInfo.alignment](
							const VkDeviceSize v
						)
						{
							return (v + alignment - 1) &
								~(alignment - 1);
						};

		const VkDeviceSize memToAllocate = getNearestOffset(rAllocInfo.size);

		if (!IndexArenaGaps.empty() &&
			IndexArenaGaps.back().getSize() >= memToAllocate)
		{
			const auto it = std::lower_bound(
				IndexArenaGaps.begin(),
				IndexArenaGaps.end(),
				memToAllocate,
				[](const clzIndexBufferMemory& iBufferMemory, const VkDeviceSize size) {
					return iBufferMemory.getSize() < size;
				}
			);

			for (auto cand = it; cand != IndexArenaGaps.end(); ++cand)
			{
				const auto nextOffset =
					getNearestOffset(cand->getMemoryOffset());

				if (cand->getEnd() < (nextOffset + memToAllocate))
					continue;

				rBufferMemory.set(nextOffset, memToAllocate);

				clzIndexBufferMemory leftGap;
				clzIndexBufferMemory rightGap;

				if (nextOffset - cand->getMemoryOffset() != 0)
				{
					leftGap.set(
						cand->getMemoryOffset(),
						nextOffset - cand->getMemoryOffset()
					);
				}
				if (cand->getEnd() > rBufferMemory.getEnd())
				{
					rightGap.set(
						rBufferMemory.getEnd(),
						cand->getEnd() - rBufferMemory.getEnd()
					);
				}

				IndexArenaGaps.erase(cand);

				if (!leftGap.isNull())
					insertIndexGap(leftGap.getMemoryOffset(), leftGap.getSize());
				if (!rightGap.isNull())
					insertIndexGap(rightGap.getMemoryOffset(), rightGap.getSize());

				return true;
			}
		}

		const VkDeviceSize newOffset = getNearestOffset(IndexBufferNewStartIndex);
		if (newOffset + memToAllocate > IndexArenaTotalSize) [[unlikely]]
		{
			clz::log::error(
				"index buffer memory is full, "
				"could not allocate more memory"
			);
			return false;
		}

		insertIndexGap(IndexBufferNewStartIndex, newOffset - IndexBufferNewStartIndex);

		rBufferMemory.set(newOffset, memToAllocate);
		IndexBufferNewStartIndex = newOffset + memToAllocate;

		return true;
	}
	/// @copydoc freeVertexBufferMemory
	void freeVertexBufferMemory(clzVertexBufferMemory& rBufferMemory)
	{
	#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			VertexBufferMemory != VK_NULL_HANDLE,
			"Vertex buffer arena memory is not initialized, "
			"but attempt was made to free memory from it."
		);
		CLZ_ASSERT(
			!rBufferMemory.isNull(),
			"freeing a null vertex buffer handle"
		);
	#endif

		/// Merge with a gap that starts exactly where this block ends.
		const auto rightIt = std::ranges::find_if(
			VertexArenaGaps,
			[rightEnd = rBufferMemory.getEnd()](
				const clzVertexBufferMemory& vBufferMemoryGap
			){
				return vBufferMemoryGap.getMemoryOffset() == rightEnd;
			}
		);
		if (rightIt != VertexArenaGaps.end())
		{
			rBufferMemory.setSize(
				rBufferMemory.getSize() + rightIt->getSize()
			);
			VertexArenaGaps.erase(rightIt);
		}

		/// Merge with a gap that ends exactly where this block starts.
		const auto leftIt = std::ranges::find_if(
			VertexArenaGaps,
			[leftEnd = rBufferMemory.getMemoryOffset()](
				const clzVertexBufferMemory& vBufferMemoryGap
			){
				return vBufferMemoryGap.getEnd() == leftEnd;
			}
		);
		if (leftIt != VertexArenaGaps.end())
		{
			rBufferMemory.set(
				leftIt->getMemoryOffset(),
				rBufferMemory.getSize() + leftIt->getSize()
			);
			VertexArenaGaps.erase(leftIt);
		}

		insertVertexGap(
			rBufferMemory.getMemoryOffset(),
			rBufferMemory.getSize()
		);

		rBufferMemory.makeNull();
	}

	/// @copydoc freeIndexBufferMemory
	void freeIndexBufferMemory(clzIndexBufferMemory& rBufferMemory)
	{
	#ifdef CLZ_ENABLE_CHECKS
		CLZ_ASSERT(
			IndexBufferMemory != VK_NULL_HANDLE,
			"Index buffer arena memory is not initialized, "
			"but attempt was made to free memory from it."
		);
		CLZ_ASSERT(
			!rBufferMemory.isNull(),
			"freeing a null index buffer handle"
		);
	#endif

		/// Merge with a gap that starts exactly where this block ends.
		const auto rightIt = std::ranges::find_if(
			IndexArenaGaps,
			[rightEnd = rBufferMemory.getEnd()](
				const clzIndexBufferMemory& iBufferMemoryGap
			){
				return iBufferMemoryGap.getMemoryOffset() == rightEnd;
			}
		);
		if (rightIt != IndexArenaGaps.end())
		{
			rBufferMemory.setSize(
				rBufferMemory.getSize() + rightIt->getSize()
			);
			IndexArenaGaps.erase(rightIt);
		}

		/// Merge with a gap that ends exactly where this block starts.
		const auto leftIt = std::ranges::find_if(
			IndexArenaGaps,
			[leftEnd = rBufferMemory.getMemoryOffset()](
				const clzIndexBufferMemory& iBufferMemoryGap
			){
				return iBufferMemoryGap.getEnd() == leftEnd;
			}
		);
		if (leftIt != IndexArenaGaps.end())
		{
			rBufferMemory.set(
				leftIt->getMemoryOffset(),
				rBufferMemory.getSize() + leftIt->getSize()
			);
			IndexArenaGaps.erase(leftIt);
		}

		insertIndexGap(
			rBufferMemory.getMemoryOffset(),
			rBufferMemory.getSize()
		);

		rBufferMemory.makeNull();
	}

	/// @copydoc bindVertexBufferToMemory
	void bindVertexBufferToMemory(
		VkBuffer& rBuffer,
		const clzVertexBufferMemory& rBufferMemory
	)
	{
		vkBindBufferMemory(
			r_deviceContext.device,
			rBuffer,
			VertexBufferMemory,
			rBufferMemory.getMemoryOffset()
		);
	}
	/// @copydoc bindIndexBufferToMemory
	void bindIndexBufferToMemory(
		VkBuffer& rBuffer,
		const clzIndexBufferMemory& rBufferMemory
	)
	{
		vkBindBufferMemory(
			r_deviceContext.device,
			rBuffer,
			IndexBufferMemory,
			rBufferMemory.getMemoryOffset()
		);
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
				&UBOArenaMemory
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
			reinterpret_cast<void**>(&UBOArenaMapped)
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
			rUBOMemoryHandle.isNull(),
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

	void bindUniformBufferToMemory(
		VkBuffer& rBuffer,
		const clzUBOMemory& rUBOMemoryHandle
	)
	{
		vkBindBufferMemory(
			r_deviceContext.device,
			rBuffer,
			UBOArenaMemory,
			rUBOMemoryHandle.getMemoryOffset()
		);
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
			return properties.limits.minStorageBufferOffsetAlignment;
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
			    reinterpret_cast<void**>(&SSBOArenaMapped)
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
			"ssbo size must be non-zero"
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

	void bindStorageBufferToMemory(
		VkBuffer& rBuffer,
		const clzSSBOMemory& rSSBOMemoryHandle
	)
	{
		vkBindBufferMemory(
			r_deviceContext.device,
			rBuffer,
			SSBOArenaMemory,
			rSSBOMemoryHandle.getMemoryOffset()
		);
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


	/// @copydoc carveImageMemory
	bool carveImageMemory(
		const ImageMemoryAllocInfo& rAllocInfo,
		clzImageMemory& rImageMemory
	)
	{
#ifdef CLZ_ENABLE_CHECKS /*****************************************/
		CLZ_ASSERT(
			rImageMemory.isNull(),
			"Attempt to carve memory to a non-null handle"
		);
#endif /***********************************************************/

		rImageMemory.m_imageType = rAllocInfo.imageType;
		auto& arena = 	ImageMemoryArenas[
					static_cast<size_t>(
						rAllocInfo.imageType
					)
				];

		if (!arena.carveMemory(rAllocInfo, rImageMemory))
			return false;

		return true;
	}

	/// @copydoc freeImageMemory
	void freeImageMemory(clzImageMemory& rImageMemory)
	{

#ifdef CLZ_ENABLE_CHECKS /*****************************************/
		CLZ_ASSERT(
			!rImageMemory.isNull(),
			"Cannot free a null image memory handle"
		);
#endif /***********************************************************/

		ImageMemoryArenas[static_cast<size_t>(
			rImageMemory.m_imageType)].freeMemory(
				rImageMemory
			);
	}


	/// @copydoc bindImageMemory
	void bindImageMemory(
		VkImage& rImage,
		const clzImageMemory& rImageMemory
	)
	{
#ifdef CLZ_ENABLE_CHECKS /***************************************************/
		CLZ_ASSERT(
			!rImageMemory.isNull(),
			"Attempt to bind null memory handle to an image"
		);
#endif /*********************************************************************/

		const auto& imageArena =
			ImageMemoryArenas[static_cast<size_t>(rImageMemory.m_imageType)];
		vkBindImageMemory(
			r_deviceContext.device,
			rImage,
			imageArena.getRawMemoryHandle(),
			rImageMemory.getMemoryOffset()
		);
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
			.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
			.pNext = nullptr
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
		if (rGap.getEnd() == m_newStartIndex)
		{
			m_newStartIndex = rGap.getMemoryOffset();
			return;
		}
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
		if (m_totalMemory < (newStartIndex + memToAllocate))
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
