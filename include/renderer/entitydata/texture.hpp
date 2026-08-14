/**
 * @file texture.hpp
 * @author curl0z
 * @brief Texture registration, upload and sampling utilities.
 */

#pragma once

#include <filesystem>
#include <limits>
#include <stb_image.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace clz::renderer
{
	struct Texture
	{
		std::vector<VkImage> image;
		std::vector<VkImageView> imageView;
		std::vector<VkDeviceSize> imageSize;
		std::vector<VkFormat> imageFormat;
		std::vector<VkDeviceSize> offset;
		std::vector<int32_t> width;
		std::vector<int32_t> height;
		std::vector<int32_t> numChannels;

		std::vector<stbi_uc*> raw_data;
	};
} // namespace clz::renderer

namespace clz::renderer
{

	constexpr uint32_t r_MAX_TEXTURE_COUNT = 1024;
	constexpr uint32_t r_NULL_TEXTURE = std::numeric_limits<uint32_t>::max();
	struct TextureID
	{
		uint32_t value = r_NULL_TEXTURE;

		TextureID() = default;
		void operator=(const TextureID& textureId)
		{
			value = textureId.value;
		}
		void operator=(const uint32_t Id)
		{
			value = Id;
		}

		[[nodiscard]] bool operator==(const TextureID& textureId) const
		{
			return value == textureId.value;
		}
		[[nodiscard]] bool operator==(const uint32_t Id) const
		{
			return value == Id;
		}

		[[nodiscard]] bool operator!=(const TextureID& textureId) const
		{
			return value != textureId.value;
		}
		[[nodiscard]] bool operator!=(const uint32_t Id) const
		{
			return value != Id;
		}
	};

	inline Texture r_textures;
	inline uint32_t r_numRegisteredTextures = 0;
	inline VkDeviceMemory r_textureDeviceMemory;
	inline VkSampler r_sampler;

	/**
	 * @brief Initializes texture subsystem state.
	 * @note Right now we create only a single global texture sampler.
	 *
	 * @return True on success.
	 */
	bool initTextureEngine();

	/**
	 * @brief Registers a texture for later GPU upload.
	 *
	 * Loads image data from disk and creates a Vulkan image handle.
	 * Actual GPU upload occurs during createTextures().
	 *
	 * @param filePath Path to texture file.
	 * @param imageFormat Format in which image will be created
	 * @return Registered texture identifier.
	 */
	TextureID registerTexture(
		const std::filesystem::path& filePath,
		VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

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
	TextureID registerTexture(
		const std::byte* data,
		size_t size,
		std::string_view textureName,
		VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

	/**
	 * @brief Uploads all registered textures to GPU memory.
	 *
	 * Creates staging buffers, allocates image memory, creates image views
	 * and performs transfer operations.
	 *
	 * @return True on success.
	 */
	bool createTextures();

	/**
	 * @brief Destroys all uploaded textures.
	 */
	void destroyTextures();

	/**
	 * @brief Creates the global texture sampler.
	 * @return True on success.
	 */
	bool createSampler();

	/**
	 * @brief Destroys the global texture sampler.
	 */
	void destroySampler();
} // namespace clz::renderer