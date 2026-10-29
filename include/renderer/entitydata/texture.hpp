/**
 * @file texture.hpp
 * @author curl0z
 * @brief Texture registration, upload and sampling utilities.
 */

#pragma once

#include "renderer/utility/image.hpp"
#include <filesystem>
#include <stb_image.h>
#include <vector>
#include <vulkan/vulkan.h>
#include "renderer/arena.hpp"
#include "core/id_interface.hpp"
#include <expected>
#include <vulkan/vulkan_core.h>

namespace clz::renderer
{
	class TextureId : public IdInterface<uint32_t>{};
	class SamplerId : public IdInterface<uint32_t>{};

	constexpr inline uint16_t MAX_TEXTURE_COUNT = 1024;
	constexpr inline uint16_t MAX_SAMPLER_COUNT = 1024;

	struct TexureData
	{
		std::string textureName;

		uint32_t width;
		uint32_t height;

		VkFormat imageFormat;
		uint32_t mipCount;
	};

	struct SamplerData
	{
		SamplerFilter magFilter;
		SamplerFilter minFilter;
		MipmapMode mipmapMode;
		SamplerAddressMode addressModeU;
		SamplerAddressMode addressModeV;
		SamplerAddressMode addressModeW;
	};

	struct TextureCPUData
	{
		TextureId stagingId;
		uint64_t textureSize = 0;
		VkDeviceSize memoryOffset = 0;
		stbi_uc* cpuData = nullptr;
		VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	};
	/// Global texture manager class
	class TextureManagerClass
	{
		/// These three are indexed by TextureId
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;
		std::vector<clzImageMemory> m_memories;
		std::vector<TexureData> m_textureDatas;

		/// This is indexed by SamplerId
		std::vector<VkSampler> m_samplers;
		std::vector<SamplerData> m_samplerDatas;
		SamplerId m_defaultSamplerId;

		std::vector<TextureId> m_textureGaps;
		std::vector<SamplerId> m_samplerGaps;

		uint32_t m_newTextureStartIndex = 0;
		uint32_t m_newSamplerStartIndex = 0;

		bool addTextureRegistry(
			TextureCPUData& cpuData,
			const std::string& textureName,
			stbi_uc* imageData,
			const uint32_t width,
			const uint32_t height
		);

	public:

		void initialize();

		std::expected<TextureCPUData, std::string> registerTexture(
			const std::filesystem::path& filePath
		);
		/**
		 * @brief Same as normal register texture function,
		 * but instead of loading them, this one takes image data
		 * as raw bytes. Aka already loaded in memory
		 *
		 * @param data Raw data, majorly used when loading GLB models
		 * @param size Size of the raw data. So that we don't go out of bounds
		 * @param textureName Name of this texture
		 * @param imageFormat Format in which image will be created
		 * @return Registered texture identifier
		 */
		std::expected<TextureCPUData, std::string> registerTexture(
			const std::byte* data,
			const size_t size,
			const std::string& textureName
		);

		SamplerId createTextureSampler(
			const SamplerData& samplerData,
			const std::string& samplerName
		);

		SamplerId getDefaultSamplerId();

		void destroyTexture(
			const TextureId textureId,
			const bool immediateDestroy = false
		);

		void destroyTextureSampler(
			const SamplerId samplerId
		);

		void destroyAllTextures();

		void destroyAllTextureSamplers();

		bool recordSubmitTextureDataToGPUCommands(
			TextureCPUData& cpuData,
			VkCommandBuffer commandBuffer,
			VkBuffer stagingBuffer,
			VkDeviceMemory stagingBufferMemory,
			const VkDeviceSize memoryOffset,
			const VkDeviceSize bufferOffset
		);

		uint32_t getTextureArrayLimit();
		uint32_t getSamplerArrayLimit();
		VkSampler getSampler(const SamplerId samplerId);
		VkImageView getImageView(const TextureId textureId);

	};

	inline TextureManagerClass TextureManager;
} // namespace clz::renderer
