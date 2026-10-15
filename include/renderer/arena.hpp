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
		VkDeviceSize vertexBufferArenaSize = 200 * 1024 * 1024;
		///< @brief Index buffer's memory size.
		VkDeviceSize indexBufferArenaSize = 100 * 1024 * 1024;
	
		///< Memory to be allocated for UBO's.
		VkDeviceSize UBOArenaSize = 100 * 1024 * 1024;

		///< Memory to be allocated for SSBO's.
		VkDeviceSize SSBO = 100 * 1024 * 1024;

		///< Memory to be allocated for textures.
		VkDeviceSize AssetTexturesArenaize = 1024 * 1024 * 1024;
		///< Memory to be allocated for internal image handles.
		VkDeviceSize ImagesArenaSize = 500 * 1024 * 1024;

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


	///< @brief Every arena's main memory handle.
	///< Each memory handle needs to simply publicly inherit this.
	class ArenaMemoryHandle
	{
	private:
		///< @brief This is the offset, 
		///< or the index in main arena's memory from which
		///< this handle's memory starts. Ofc in bytes.
		VkDeviceSize memoryOffset = std::numeric_limits<VkDeviceSize>::max();
		///< @brief This is the total size of allocated 
		///< to this memory handle, in bytes.
		VkDeviceSize size = 0;

	public:
		ArenaMemoryHandle() = default;
		ArenaMemoryHandle(
			const VkDeviceSize memoryOffset, 
			const VkDeviceSize size
		) : 
			memoryOffset(memoryOffset),
			size(size)
		{
		}

		/// @brief Offset in bytes into the arena's memory.
		[[nodiscard]] VkDeviceSize getMemoryOffset() const
		{
			return memoryOffset;
		}
		/// @brief Size in bytes of the allocation.
		[[nodiscard]] VkDeviceSize getSize() const
		{
			return size;
		}
		/// @brief One past the last byte (offset + size).
		/// Handy for the adjacency checks in free.
		/// @warn Never to be called outside any arena function.
		[[nodiscard]] VkDeviceSize getEnd() const
		{
			return memoryOffset + size;
		}

		/**
		 * @brief Sets new offset for this handle.
		 * @param newOffset New offset of this handle.
		 * @warn Never to be called outside any arena function.
		 */
		void setMemoryOffset(const VkDeviceSize newOffset)
		{
			memoryOffset = newOffset;
		}
		/**
		 * @brief Sets new size for this handle.
		 * @param newSize New size if this handle.
		 * @warn Never to be called outside any arena function.
		 */
		void setSize(const VkDeviceSize newSize)
		{
			size = newSize;
		}
		/**
		 * @brief Sets new offset and size for this handle.
		 * @param newOffset New offset of this handle.
		 * @param newSize New size if this handle.
		 * @warn Never to be called outside any arena function.
		 */
		void set(const VkDeviceSize newOffset, const VkDeviceSize newSize)
		{
			memoryOffset = newOffset;
			size = newSize;
		}

		/**
		 * @brief Queries whether this handle is null or not
		 */
		[[nodiscard]] bool isNull() const
		{
			return memoryOffset == std::numeric_limits<VkDeviceSize>::max() &&
				size == 0;
		}
		/**
		 * @brief Makes this handle null.
		 * @warn Never to be called outside any arena function.
		 */
		void makeNull()
		{
			memoryOffset = std::numeric_limits<VkDeviceSize>::max();
			size = 0;
		}

	};
	

	/******************************************************/
	/********* 0. VERTEX BUFFER AND INDEX BUFFER **********/
	/******************************************************/

	///< @brief Main Vertex buffer memory handle.
	inline VkDeviceMemory VertexBufferMemory;
	///< @brief Main Index buffer memory handle.
	inline VkDeviceMemory IndexBufferMemory;
	///< @brief Main vertex buffer handle.
	inline VkBuffer VertexBuffer;
	///< @brief Main index buffer handle.
	inline VkBuffer IndexBuffer;
	///< @brief Vertex arena total memory size.
	inline VkDeviceSize VertexArenaTotalSize;
	///< @brief Index buffer total memory size.
	inline VkDeviceSize IndexArenaTotalSize;
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
	 * @param vertexBufferArenaSize Size of vertex buffer arena in bytes.
	 * @param indexBufferArenaSize Size of index buffer arena in bytes.
	 * @return bool indicating completion status
	 */
	bool allocateBufferArena(
		const VkDeviceSize vertexBufferArenaSize,
		const VkDeviceSize indexBufferArenaSize);
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
	 * @brief Allocates index buffer from main memory.
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
		clzIndexBuffer* pIndexBuffer
	);



	/**************************/
	/********* 1. UBO *********/
	/**************************/

	///< @brief Main UBO memory handle.
	inline VkDeviceMemory UBOArenaMemory;
	///< @brief UBO memory total size.
	inline VkDeviceSize UBOArenaTotalSize;
	///< @brief UBOMemoryNewStartIndex.
	inline VkDeviceSize UBOMemoryNewStartIndex;

	/**
	 * @brief Allocates UBO arena memory.
	 * @param uboArenaSize Size to allocate for UBO's arena.
	 * @return bool indicating completion status.
	 */
	bool allocateUBOArenaMemory(
		const VkDeviceSize uboArenaSize
	);
	/**
	 * @brief Destroys ubo's main memory
	 */
	void destroyUBOArenaMemory();

	///< @brief UBO Memory allocation info
	typedef struct UBOMemoryAllocInfo
	{
		///< @brief Total size to allocate.
		VkDeviceSize size = 0;
		///< @brief Alignment in memory.
		VkDeviceSize alignment = 0;
	} UBOMemoryAllocInfo;
	///< @brief engine's internal ubo memory handle.
	class clzUBOMemory : public ArenaMemoryHandle{};
	///< @brief Stores all gaps in main UBO's arena memory.
	///< @note this vector is always sorted by size, not offset.
	inline std::vector<clzUBOMemory> UBOArenaGaps;

	/**
	 * @brief Carves Allocates UBO memory from arena.
	 * @param rAllocInfo const reference to ubo memory allocation info.
	 * @param rUBOMemoryHandle reference to handle which will hold this memory.
	 * @note The size that this handle owns might be greater than mentioned in rAllocInfo.size,
	 * because of alignment constraints.
	 * @return true on success, false otherwise.
	 */
	bool carveUBOMemory(
		const UBOMemoryAllocInfo& rAllocInfo,
		clzUBOMemory& rUBOMemoryHandle
	);
	/**
	 * @brief Frees UBO memory of a handle.
	 * @param rUBOMemoryHandle reference to ubo memory handle.
	 * @note Passed handle is null after calling this function, 
	 * so make sure to not use it after this.
	 */
	void freeUBOMemory(
		clzUBOMemory& rUBOMemoryHandle
	);

}
