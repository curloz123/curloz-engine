/**
 * @file arena.hpp
 * @author curl0z
 * @brief Vulkan memory arena allocator
 * @note For buffers, this file provides clzVertexBuffer and clzIndexBuffer that
 * are full on handles, not just memory handles. Everything else is simply
 * a memory handle. So externally there needs be some other work too for them.
 *
 */

#pragma once

#include <limits>
#include <vulkan/vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace clz::renderer
{
	///< @brief Defines memory creation data	
	typedef struct VkArenaDef
	{
		///< @brief Vertex buffer's memory size.
		VkDeviceSize vertexBufferMemSize = 200 * 1024 * 1024;
		///< @brief Index buffer's memory size.
		VkDeviceSize indexBufferMemSize = 100 * 1024 * 1024;
	
		///< Memory to be allocated for UBO's.
		VkDeviceSize UBO = 100 * 1024 * 1024;

		///< Memory to be allocated for SSBO's.
		VkDeviceSize SSBO = 100 * 1024 * 1024;

		///< Memory to be allocated for textures.
		VkDeviceSize AssetTexturesMemSize = 1024 * 1024 * 1024;
		///< Memory to be allocated for internal image handles.
		VkDeviceSize ImagesMemSize = 500 * 1024 * 1024;

	} VkArenaDef;

	/**
	 * @brief Allocates vulkan arena's their respective memory.
	 * @param arenaDef Arena's definition.
	 * @return bool indicating success status.
	 * @note Device context must've been initialized before calling this.
	 */
	bool allocateVkArenaMemory(const VkArenaDef& arenaDef);
	
	/**
	 * @brief Destroy's all arena's memory
	 */
	void destroyVkArenaMemory();
	


	///< @brief Main Vertex buffer memory handle.
	inline VkDeviceMemory VertexBufferMemory;
	///< @brief Main Index buffer memory handle.
	inline VkDeviceMemory IndexBufferMemory;
	///< @brief Main vertex buffer handle.
	inline VkBuffer VertexBuffer;
	///< @brief Main index buffer handle.
	inline VkBuffer IndexBuffer;
	///< @brief Vertex buffer total memory size.
	inline VkDeviceSize VertexBufferSize;
	///< @brief Index buffer total memory size.
	inline VkDeviceSize IndexBufferSize;
	///< @brief Memory offset of vertex buffer in memory.
	inline VkDeviceSize VertexBufferMemoryOffset;
	///< @brief Memory offset of index buffer in memory.
	inline VkDeviceSize IndexBufferMemoryOffset;
	///< @brief Vertex buffer alloc offset.
	///< Basically offset in memory from where
	///< we can allocate new stuff.
	inline VkDeviceSize VertexBufferNewStartIndex;
	///< @brief Index buffer alloc offset.
	///< Basically offset in memory from where
	///< we can allocate new stuff.
	inline VkDeviceSize IndexBufferNewStartIndex;

	/**
	 * @brief Allocates buffer memory
	 * @param vertexBufferSize Size of vertex buffer in bytes.
	 * @param indexBufferSize Size of index buffer in bytes.
	 * @return bool indicating completion status
	 */
	bool allocateBufferArena(
		const VkDeviceSize vertexBufferSize,
		const VkDeviceSize indexBufferSize);
	/*
	 * @brief Destroys main buffer
	 */
	void destroyBufferArena();
	
	/**
	 * @brief Binds main vertex buffer at main draw call once.
	 * @note Only needs to be called once before main drawing
	 * @param commandBuffer Active command buffer while recording.
	 */
	inline void bindGlobalVertexBuffer(
		const VkCommandBuffer commandBuffer
	)
	{
		constexpr VkDeviceSize offsets[1] = {0};
		const VkBuffer buffers[1] = {VertexBuffer};

		vkCmdBindVertexBuffers(
			commandBuffer,
			0,
			1,
			buffers,
			offsets
		);
	}
	/**
	 * @brief Binds Index buffer at main draw call once.
	 * @note Needs be called only once during main call.
	 * @param commmandBuffer Active command buffer during recording.
	 */
	inline void bindGlobalIndexBuffer(
		const VkCommandBuffer commandBuffer
	)
	{
		vkCmdBindIndexBuffer(
			commandBuffer, 
			IndexBuffer, 
			0, 
			VK_INDEX_TYPE_UINT32
		);
	}

	///< @brief CLZ engine's main vertex buffer handle.
	typedef struct clzVertexBuffer
	{
		///< @brief This is the offset, 
		///< or the index in main buffer memory from which
		///< this buffer starts. Ofc in bytes.
		VkDeviceSize memoryOffset = std::numeric_limits<VkDeviceSize>::max();
		///< @brief This is the total size of this 
		///< vertex buffer, in bytes.
		VkDeviceSize size = 0;
	} clzVertexBuffer;
	///< @brief CLZ engine's main index buffer handle
	typedef struct clzIndexBuffer
	{
		///< @brief Memory index from where this index buffer starts.
		VkDeviceSize memoryOffset = std::numeric_limits<VkDeviceSize>::max();
		///< @brief Number of indices this buffer holds.
		VkDeviceSize size = 0;
	} clzIndexBuffer;

	///< @brief Vertex buffer allocation info.
	typedef struct VertexBufferAllocInfo
	{
		///< @brief Total size to allocate.
		VkDeviceSize size = 0;
	} VertexBufferAllocInfo;
	///< @brief Index Buffer allocation info.
	typedef struct IndexBufferAllocInfo
	{
		///< @brief Total size to allocate.
		VkDeviceSize size = 0;
	} IndexBufferAllocInfo;

	///< @brief Stores all gaps in vertex buffer.
	///< Basically when we free from in between,
	///< and we do any new allocation, we won't miss the gaps in bw
	///< @note This vector should always be in ascended sorted,
	///< ordered by size.
	inline std::vector<clzVertexBuffer> VertexBufferGaps;
	///< @brief Stores all gaps in index buffer.
	///< @note This vector should always be in ascended sorted,
	///< ordered by size.
	inline std::vector<clzIndexBuffer> IndexBufferGaps;

	/**
	 * @brief Allocates vertex buffer from main memory.
	 * @param pAllocInfo Pointer to vertex buffer allocation info.
	 * @param pVertexBuffer pointer to engine's internal v buffer handle.
	 * @return true on success, false otherwise.
	 */
	bool allocateVertexBuffer(
		const VertexBufferAllocInfo* pAllocInfo,
		clzVertexBuffer* pVertexBuffer
	);
	/**
	 * @brief Allocates index from main memory.
	 * @param pAllocInfo Pointer to index buffer allocation info.
	 * @param pIndexBuffer pointer to engine's internal i buffer handle.
	 * @return true on success, false otherwise.
	 */
	bool allocateIndexBuffer(
		const IndexBufferAllocInfo* pAllocInfo,
		clzIndexBuffer* pIndexBuffer
	);

	/**
	 * @brief Frees memory from main vertex buffer memory
	 * @param pVertexBuffer pointer to vertex buffer handle
	 * @note Is not a cheap process tbh. Internally linear searches 
	 * through all gaps to check for adjacent memories.
	 */
	void freeVertexBuffer(
		clzVertexBuffer* pVertexBuffer
	);
	/**
	 * @brief Frees memory from main index buffer memory
	 * @param pIndexBuffer pointer to index buffer handle
	 * @note Is not a cheap process tbh. Internally linear searches 
	 * through all gaps to check for adjacent memories.
	 */
	void freeIndexBuffer(
		clzVertexBuffer* pIndexBuffer
	);


}
