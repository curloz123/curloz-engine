#include "renderer/entitydata/texture.hpp"
#include "core/logs.hpp"
#include "renderer/arena.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/vk_types.hpp"
#include <cfloat>
#include <expected>
#include <filesystem>
#include <memory.h>
#include <stb_image.h>
#include <vulkan/vulkan_core.h>

namespace clz::renderer
{
	void TextureManagerClass::initialize()
	{
		m_images.clear();
		m_imageViews.clear();
		m_memories.clear();
		m_textureDatas.clear();

		m_samplers.clear();
		m_samplerDatas.clear();

		/// Create a default sampler
		m_defaultSamplerId = SamplerId(0);
		m_samplers.resize(1);
		m_samplerDatas.resize(1);
		m_samplerDatas[0] = {
			.magFilter = SamplerFilter::Linear,
			.minFilter = SamplerFilter::Linear,
			.mipmapMode = MipmapMode::Linear,
			.addressModeU = SamplerAddressMode::REPEAT,
			.addressModeV = SamplerAddressMode::REPEAT,
			.addressModeW = SamplerAddressMode::REPEAT
		};
		if (!createSampler(
			m_samplers[0],
			"textures default sampler",
			SamplerFilter::Linear,
			SamplerFilter::Linear,
			MipmapMode::Linear,
			SamplerAddressMode::REPEAT,
			SamplerAddressMode::REPEAT,
			SamplerAddressMode::REPEAT,
			FLT_MAX,
			true,
			16.0
		))
		{
			clz::log::error("Could not create default sampler", true);
		}

		m_textureGaps.clear();
		m_samplerGaps.clear();

		m_newTextureStartIndex = 0;
		m_newSamplerStartIndex = 1;
	}

	bool TextureManagerClass::addTextureRegistry(
		TextureCPUData& cpuData,
		const std::string& textureName,
		stbi_uc* imageData,
		const uint32_t width,
		const uint32_t height
	)
	{
		auto getTextureId = [&](){
			if (!m_textureGaps.empty())
			{
				TextureId newId = m_textureGaps.back();
				m_textureGaps.pop_back();
				return newId;
			}

			if (m_newTextureStartIndex >= MAX_TEXTURE_COUNT)
			{
				clz::log::error("Max texture limit reached");
				return TextureId();
			}

			TextureId newId(m_newTextureStartIndex);
			++m_newTextureStartIndex;
			m_images.resize(m_newTextureStartIndex);
			m_imageViews.resize(m_newTextureStartIndex);
			m_memories.resize(m_newTextureStartIndex);
			m_textureDatas.resize(m_newTextureStartIndex);

			return newId;
		};

		const auto textureId = getTextureId();
		if (textureId.isNull())
			return false;

		m_textureDatas[textureId.getId()] = {
			.textureName = textureName,
			.width = width,
			.height = height,
			/// Format is left blank
			.mipCount = calculateMipLevels(width, height)
		};

		cpuData = TextureCPUData{
			.stagingId = textureId,
			.textureSize = width * height * STBI_rgb_alpha,
			.cpuData = imageData,
		};

		return true;
	}
	std::expected<TextureCPUData, std::string> TextureManagerClass::registerTexture(
		const std::filesystem::path& filePath
	)
	{
		if (!std::filesystem::exists(filePath))
		{
			clz::log::warn(filePath.string() + " does not exist");
			return std::unexpected(filePath.string() + " doesn't exists");
		}

		int width, height, numChannels;
		stbi_uc* rawData = stbi_load(
					filePath.string().c_str(),
					&width,
					&height,
					&numChannels,
					STBI_rgb_alpha
				);
		if (!rawData)
		{
			return std::unexpected("Error loading texture: " + filePath.string());
		}

		TextureCPUData cpuData;
		if (!addTextureRegistry(
			cpuData,
			filePath.string(),
			rawData,
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		))
		{
			stbi_image_free(rawData);
			return std::unexpected("Error registering texure: " + filePath.string());
		}

		return cpuData;
	}

	std::expected<TextureCPUData, std::string> TextureManagerClass::registerTexture(
		const std::byte* data,
		size_t size,
		const std::string& textureName
	)
	{
		int width, height, numChannels;
		stbi_uc* rawData = stbi_load_from_memory(
			reinterpret_cast<const stbi_uc*>(data),
			static_cast<int>(size),
			&width,
			&height,
			&numChannels,
			STBI_rgb_alpha
		);
		if (!rawData)
		{
			return std::unexpected("Error loading: " + textureName);
		}

		TextureCPUData cpuData;
		if (!addTextureRegistry(
			cpuData,
			textureName,
			rawData,
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		))
		{
			stbi_image_free(rawData);
			return std::unexpected("Error registering texure: " + textureName);
		}

		return cpuData;
	}
	SamplerId TextureManagerClass::createTextureSampler(
		const SamplerData& samplerData,
		const std::string& samplerName
	)
	{
		auto getSamplerId = [&](){
			if (!m_samplerGaps.empty())
			{
				SamplerId newId = m_samplerGaps.back();
				m_samplerGaps.pop_back();
				return newId;
			}

			SamplerId newId(m_newSamplerStartIndex);
			++m_newSamplerStartIndex;
			m_samplers.resize(m_newSamplerStartIndex);
			m_samplerDatas.resize(m_newSamplerStartIndex);

			return newId;
		};

		SamplerId newId = getSamplerId();
		auto& sampler = m_samplers[newId.getId()];
		/// Then sampler
		if (!createSampler(
			sampler,
			samplerName,
			samplerData.magFilter,
			samplerData.minFilter,
			samplerData.mipmapMode,
			samplerData.addressModeU,
			samplerData.addressModeV,
			samplerData.addressModeW,
			FLT_MAX,
			true,
			16.0f
		))
		{
			clz::log::error(
				"Vulkan could not create sampler: " +
				samplerName
			);
			return m_defaultSamplerId;
		}

		m_samplerDatas[newId.getId()] = samplerData;

		return newId;
	}

	SamplerId TextureManagerClass::getDefaultSamplerId()
	{
		return m_defaultSamplerId;
	}

	void TextureManagerClass::destroyTexture(
		const TextureId textureId,
		const bool immediateDestroy
	)
	{
#ifdef CLZ_ENABLE_CHECKS /***********************************************************/
		if (textureId.isNull())
		{
			clz::log::warn("Attempt to destroy a null texture was done");
			return;
		}
#endif /*****************************************************************************/

		const auto index = textureId.getId();
		const auto& device = r_deviceContext.device;
		vkDestroyImageView(

			device,
			m_imageViews[index],
			nullptr
		);
		vkDestroyImage(
			device,
			m_images[index],
			nullptr
		);
		if (immediateDestroy)
		{
			freeImageMemory(m_memories[index]);
			m_textureGaps.push_back(textureId);
		}

		m_imageViews[index] = VK_NULL_HANDLE;
		m_images[index] = VK_NULL_HANDLE;
	}

	void TextureManagerClass::destroyTextureSampler(
		const SamplerId samplerId
	)
	{
		vkDestroySampler(
			r_deviceContext.device,
			m_samplers[samplerId.getId()],
			nullptr
		);
		m_samplers[samplerId.getId()] = VK_NULL_HANDLE;
		m_samplerGaps.push_back(samplerId);
	}

	void TextureManagerClass::destroyAllTextures()
	{
		for (size_t i = 0; i < m_images.size(); ++i)
			destroyTexture(TextureId(i));
		m_textureDatas.clear();
	}

	void TextureManagerClass::destroyAllTextureSamplers()
	{
		for (uint32_t i = 0; i < m_samplers.size(); ++i)
			destroyTextureSampler(SamplerId(i));
		m_samplerDatas.clear();
	}

	bool TextureManagerClass::recordSubmitTextureDataToGPUCommands(
		TextureCPUData& cpuData,
		VkCommandBuffer commandBuffer,
		VkBuffer stagingBuffer,
		VkDeviceMemory stagingBufferMemory,
		const VkDeviceSize memoryOffset,
		const VkDeviceSize bufferOffset
	)
	{
#ifdef CLZ_ENABLE_CHECKS /*****************************************************************/
		CLZ_ASSERT(
			cpuData.cpuData,
			"Null texture data passed while submitting data to GPU"
		);
		CLZ_ASSERT(
			!cpuData.stagingId.isNull(),
			"Staging texture was'nt registered before submitting data to GPU"
		);
		CLZ_ASSERT(
			stagingBuffer != VK_NULL_HANDLE,
			"Null staging buffer passed while submit texture data to gpu"
		);
		CLZ_ASSERT(
			stagingBufferMemory != VK_NULL_HANDLE,
			"Null staging buffer passed while submit texture data to gpu"
		);
#endif /***********************************************************************************/

		const auto& index = cpuData.stagingId.getId();
		const auto& size = cpuData.textureSize;

		const auto& device = r_deviceContext.device;
		auto& textureData = m_textureDatas[index];
		textureData.imageFormat = cpuData.imageFormat;

		auto& image = m_images[index];
		auto& imageView = m_imageViews[index];
		auto& memory = m_memories[index];

		/// first create vulkan image handle
		if (!createImage(
			image,
			textureData.textureName,
			textureData.width,
			textureData.height,
			textureData.imageFormat,
			VK_IMAGE_TILING_OPTIMAL,
   			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      				VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
          			VK_IMAGE_USAGE_SAMPLED_BIT,
             		0,
               		textureData.mipCount
		))
		{
			clz::log::error(
				"Vulkan could not create texture: " +
					textureData.textureName
			);
			return false;
		}

		/// Now main thing, memory
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);
		const ImageMemoryAllocInfo memAllocInfo{
			.imageType = ImageType::TEXTURE,
			.alignment = memRequirements.alignment,
			.size = memRequirements.size,
		};
		if (!carveImageMemory(memAllocInfo, memory))
		{
			clz::log::error(
				"Error allocationg memory for texture: " +
					textureData.textureName
			);
			return false;
		}
		bindImageMemory(image, memory);

		/// create image view
		if (!createImageView(
			imageView,
			textureData.textureName + " image view",
			image,
			textureData.imageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT,
			textureData.mipCount
		))
		{
			clz::log::error(
				"Vulkan could not create image view for: " +
					textureData.textureName
			);
			return false;
		}

		/// Now main thing, memory
		void* data;
		vkMapMemory(
			device,
			stagingBufferMemory,
			memoryOffset,
			size,
			0,
			&data
		);
		memcpy(data, cpuData.cpuData, size);
		vkUnmapMemory(device, stagingBufferMemory);
		stbi_image_free(cpuData.cpuData);
		cpuData.cpuData = nullptr;

		transition_image_layout(
			image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_ACCESS_2_NONE,
			VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			commandBuffer,
			0,
			textureData.mipCount
		);

		const VkBufferImageCopy copyRegion{
			.bufferOffset = bufferOffset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageOffset = {0, 0, 0},
			.imageExtent = {textureData.width, textureData.height, 1}
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			stagingBuffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copyRegion
		);

		auto newLayout = generateMipmaps(
			commandBuffer,
			image,
			textureData.width,
			textureData.height,
			textureData.mipCount
		);

		transition_image_layout(
			image,
			newLayout,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_2_TRANSFER_READ_BIT,
			VK_ACCESS_2_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			commandBuffer,
			0,
			textureData.mipCount
		);

		return true;
	}

	uint32_t TextureManagerClass::getTextureArrayLimit()
	{
		return m_newTextureStartIndex;
	}
	uint32_t TextureManagerClass::getSamplerArrayLimit()
	{
		return m_newSamplerStartIndex;
	}

	VkSampler TextureManagerClass::getSampler(const SamplerId samplerId)
	{
		const auto& sampler = m_samplers[samplerId.getId()];
		return sampler;
	}
	VkImageView TextureManagerClass::getImageView(const TextureId textureId)
	{
		const auto& imageView = m_imageViews[textureId.getId()];
		return imageView;
	}
} // namespace clz::renderer
