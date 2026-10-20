/**
 * @file arena.hpp
 * @author curl0z
 * @brief Vulkan memory arena allocator
 * @note For buffers, this file provides clzVertexBuffer and clzIndexBuffer that
 * are full on handles, not just memory handles. Everything else is simply
 * a memory handle. So externally there needs be some other work too for them.
 */

#pragma once

#include <limits>
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <string>
#include "core/assert.hpp"
#include "renderer/vk_types.hpp"

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
		VkDeviceSize UBOArenaSize = 10 * 1024 * 1024;
		///< Memory to be allocated for SSBO's.
		VkDeviceSize SSBOArenaSize = 20 * 1024 * 1024;

		///< Memory to be allocated for canvas images.
		VkDeviceSize CanvasImagesArenaSize = 150 * 1024 * 1024;
		///< Memory to be allocated for depth images.
		VkDeviceSize DepthImagesArenaSize = 200 * 1024 * 1024;
		///< Memory to be allocated for msaa images.
		VkDeviceSize MSAAImagesArenaSize = 150 * 1024 * 1024;
		///< Memory to be allocated for textures.
		VkDeviceSize TextureImagesArenaSize = 1024 * 1024 * 1024;


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
	public:
		///< @brief This is the offset, 
		///< or the index in main arena's memory from which
		///< this handle's memory starts. Ofc in bytes.
		VkDeviceSize memoryOffset = std::numeric_limits<VkDeviceSize>::max();
		///< @brief This is the total size of allocated 
		///< to this memory handle, in bytes.
		VkDeviceSize size = 0;

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
	inline VkDeviceMemory UBOArenaMemory = 0;
	///< @brief Mapped pointer to UBO arena memory.
	inline std::byte* UBOArenaMapped = nullptr;
	///< @brief UBO memory total size.
	inline VkDeviceSize UBOArenaTotalSize = 0;
	///< @brief UBOMemoryNewStartIndex.
	inline VkDeviceSize UBOMemoryNewStartIndex = 0;

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
	 * @note It is always made sure that every ubo respects ubo alignments.
	 * for that reasons it might store more memory than it needs to.
	 * This is done so that any new allocations can be started easily.
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

	/**
	 * @brief Retrieves vulkan's mapped pointer to a uniform buffer object.
	 * @param rUBOMemoryHandle Const reference to UBO memory handle.
	 * @retrurn std::byte* pointing towards memory.
	 */
	inline std::byte* getUBOMappedPointer(const clzUBOMemory& rUBOMemoryHandle)
	{
		return UBOArenaMapped + rUBOMemoryHandle.getMemoryOffset();
	}

	/***************************/
	/********* 2. SSBO *********/
	/***************************/

	///< @brief Main SSBO memory handle.
	inline VkDeviceMemory SSBOArenaMemory = VK_NULL_HANDLE;
	///< @brief Mapped pointer to the whole SSBO arena.
	inline std::byte* SSBOArenaMapped = nullptr;
	///< @brief SSBO arena total size in bytes.
	inline VkDeviceSize SSBOArenaTotalSize = 0;
	///< @brief Offset from which fresh (never used) memory can be carved.
	inline VkDeviceSize SSBOMemoryNewStartIndex = 0;

	/**
	 * @brief Allocates SSBO arena memory and maps it.
	 * @param ssboArenaSize Size to
	 * allocate for the SSBO arena.
	 * @return true on success, false otherwise.
	 */
	bool allocateSSBOArenaMemory(const VkDeviceSize ssboArenaSize);
	/**
	 * @brief Unmaps and frees SSBO arena memory, resets all arena state.
	 */
	void destroySSBOArenaMemory();

	///< @brief SSBO memory allocation info.
	typedef struct SSBOMemoryAllocInfo
	{
		///< @brief Size to allocate.
		VkDeviceSize size = 0;
		///< @brief Bind alignment.
		VkDeviceSize alignment = 0;
	} SSBOMemoryAllocInfo;
	///< @brief engine's internal ssbo memory handle.
	class clzSSBOMemory : public ArenaMemoryHandle{};
	///< @brief All free gaps in the SSBO arena, always sorted by size.
	inline std::vector<clzSSBOMemory> SSBOArenaGaps;

	/**
	 * @brief Carves SSBO memory from the arena.
	 * @param rAllocInfo Size and alignment of the buffer to be bound here.
	 * @param rSSBOMemoryHandle Handle that will own the carved block.
	 * @note The block's size is rounded up to the alignment, so it can be
	 * larger than rAllocInfo.size.
	 * @return true on success, false otherwise.
	 */
	bool carveSSBOMemory(
		const SSBOMemoryAllocInfo& rAllocInfo,
		clzSSBOMemory& rSSBOMemoryHandle
	);
	/**
	 * @brief Returns an SSBO block to the arena.
	 * @param rSSBOMemoryHandle Handle to free.
	 * @note Handle is null after this call.
	 * @warning Destroy the VkBuffer bound to this block before calling this.
	 */
	void freeSSBOMemory(clzSSBOMemory& rSSBOMemoryHandle);

	/**
	 * @brief Retrieves vulkan's mapped pointer to a storage buffer object.
	 * @param rSSBOMemoryHandle Const reference to SSBO memory handle.
	 * @retrurn std::byte* pointing towards memory.
	 */
	inline std::byte* getSSBOMappedPointer(const clzSSBOMemory& rSSBOMemoryHandle)
	{
		return SSBOArenaMapped + rSSBOMemoryHandle.getMemoryOffset();
	}



	/*****************************/
	/********* 3. IMAGES *********/
	/*****************************/

	/**
	 * This part is the weird one because there is
	 * not one type of memory that might be jack of
	 * all trades. That's why a generic class of arena
	 * has been made, each tending the needs of a
	 * particular type of images.
	 * As of now, this supports:
	 * 1. Canvas images (post process images,
	 *			render target,
	 *			editor images, etc.)
	 *	Basically anything which is to be used
	 *	by shaders, maybe to draw on or read etc..
	 *	MSAA images don't come under this.
	 * 2. Depth images
	 * 3. MSAA images
	 * 4. Textures of assets
	 *
	 * And there is one global (hard-coded sized) array
	 * which holds all arenas and is indexed by ImageType enum.
	 * Rest all allocation n stuff are pretty much same.
	 */

	///< @brief Types of images.
	///< These also indicate the index of image type in array.
	enum class ImageType : std::uint8_t
	{
		CANVAS		= 0,
		DEPTH		= 1,
		MULTISAMPLED	= 2,
		TEXTURE		= 3,
	};

	///< @brief Engine's internal image memory handle.
	///< @note Holds an extra functionality about which type of image it is.
	class clzImageMemory : public ArenaMemoryHandle
	{
	public:
		///< @brief Type of image.
		ImageType m_imageType;
	};

	///< Image memory allocation info struct.
	typedef struct ImageMemoryAllocInfo
	{
		///< Type of image.
		ImageType imageType;

		///< Alignment required by image.
		///< Must be non-zero.
		VkDeviceSize alignment;

		///< Size required by image.
		///< Must be non-zero.
		VkDeviceSize size;
	} ImageMemoryAllocInfo;

	///< @brief Main generic image memory arena class
	class ImageMemoryArena
	{
	private:
		///< @brief Arena name
		std::string m_arenaName;

		///< @brief Actual arena memory handle.
		VkDeviceMemory m_arenaMemory = VK_NULL_HANDLE;		

		///< @brief Total memory the arena holds.
		VkDeviceSize m_totalMemory = 0;

		///< @brief Index from where new allocation can start.
		VkDeviceSize m_newStartIndex = 0;

		///< @brief Stores all gaps in arena memory.
		std::vector<clzImageMemory> m_gaps;

		/**
		 * @brief Gaps insertion helper.
		 * Simply takes the memory handle(gap),
		 * finds the suitable index in sorted m_gaps array,
		 * and inserts there.
		 * @param rGap Const reference to memory handle.
		 */
		void insertGap(const clzImageMemory& rGap);
	public:
		ImageMemoryArena() = default;

		/**
		 * @brief Initializes the memory arena.
		 * @note All parameters you see here should denote the general format
		 * which should nearly match the format of the image for which we're
		 * allocating memory.
		 * @param arenaName Name of the arena.
		 * @param arenaSize Size of the arena.
		 * @param format General format in which you can expect these.
		 * @param mipLevels Number of mip-map levels.
		 * @param sampleBits Number of sample bits.
		 * @param tiling The tiling of the image.
		 * @param usageFlags Usage flags of image.
		 * @return true on success, false otherwise.
		 */
		bool initialize(
			const std::string& arenaName,
			VkDeviceSize arenaSize,
			VkFormat format,
			uint8_t mipLevels,
			VkSampleCountFlagBits sampleBits,
			VkImageTiling tiling,
			VkImageUsageFlags usageFlags
		);

		/**
		 * @brief Destroys the arena,
		 */
		void destroy();

		/**
		 * @brief Carves memory from the arena.
		 * @param rAllocInfo const reference to
		 * image memory allocation info struct.
		 * @param rImageMemory handle which will hold this memory.
		 * @return true on success, false otherwise.
		 */
		bool carveMemory(
			const ImageMemoryAllocInfo& rAllocInfo,
			clzImageMemory& rImageMemory
		);

		/**
		 * @brief Frees memory in arena.
		 * Is not a cheap process, so don't go gung-ho
		 * and free every frame.
		 * @param Reference to memory handle.
		 * @note The handle passed will be null after passing to this func.
		 */
		void freeMemory(clzImageMemory& rImageMemory);

		/// @brief Retrieves the raw handle to memory.
		const VkDeviceMemory& getRawMemoryHandle() const
		{
			return m_arenaMemory;
		}
	};

	///< @brief Global array holding all the image memory arenas.
	inline std::array<ImageMemoryArena, 5> ImageMemoryArenas;


	/// All below functions are just helper functions,
	/// so that we don't index the arena array everytime.


	/**
	 * @brief Carves image memory from the arena.
	 * @param imageType The type of image we're trynna create.
	 * @param rAllocInfo const reference to
	 * image memory allocation info struct.
	 * @param rImageMemory handle which will hold this memory.
	 * @return true on success, false otherwise.
	 */
	inline bool carveImageMemory(
		const ImageMemoryAllocInfo& rAllocInfo,
		clzImageMemory& rImageMemory
	)
	{
		rImageMemory.m_imageType = rAllocInfo.imageType;
		ImageMemoryArenas[static_cast<size_t>(
			rAllocInfo.imageType)].carveMemory(
				rAllocInfo,
				rImageMemory
			);
	}

	/**
	 * @brief Frees image memory in arena.
	 * Is not a cheap process, so don't go gung-ho
	 * and free every frame.
	 * @param imageType The type of image we're trynna create.
	 * @param Reference to memory handle.
	 * @note The handle passed will be null after passing to this func.
	 */
	inline void freeImageMemory(clzImageMemory& rImageMemory)
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

	/**
	 * @brief Helper function that binds memory to any image from arena.
	 * @param rImage Reference to vulkan image handle.
	 * @param rImageMemory Reference to image memory handle.
	 */
	inline void bindImageMemory(
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

}
