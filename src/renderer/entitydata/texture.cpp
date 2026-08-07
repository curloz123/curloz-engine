#include "../../../include/renderer/entitydata/texture.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "renderer/utility/buffer.hpp"
#include "renderer/utility/image.hpp"
#include "renderer/utility/memory.hpp"
#include "renderer/utility/namer.hpp"
#include "renderer/utility/offsetalignment.hpp"
#include "renderer/utility/singletimecommand.hpp"
#include "renderer/vk_types.hpp"
#include <filesystem>
#include <limits>
#include <memory.h>
#include <stb_image.h>

namespace clz::renderer
{
bool initTextureEngine()
{
	if (!createSampler())
	{
		clz::CLZ_ASSERT(false, "could not create texture sampler");
		clz::log::error("Could not create texture sampler");
		return false;
	}

	return true;
}
TextureID registerTexture(const std::filesystem::path& filePath)
{
	if (!std::filesystem::exists(filePath))
	{
		clz::log::warn(filePath.string() + " does not exist");
		return r_NULL_TEXTURE;
	}
	r_textures.image.resize(r_textures.image.size() + 1);
	r_textures.imageView.resize(r_textures.image.size() + 1);
	r_textures.imageSize.resize(r_textures.imageSize.size() + 1);
	r_textures.offset.resize(r_textures.offset.size() + 1);
	r_textures.width.resize(r_textures.image.size() + 1);
	r_textures.height.resize(r_textures.image.size() + 1);
	r_textures.numChannels.resize(r_textures.image.size() + 1);
	r_textures.raw_data.resize(r_textures.image.size() + 1);
	const uint32_t index = r_numRegisteredTextures;

	r_textures.raw_data[index] = nullptr;
	r_textures.raw_data[index] = stbi_load(
		filePath.c_str(),
		&r_textures.width[index],
		&r_textures.height[index],
		&r_textures.numChannels[index],
		STBI_rgb_alpha
	);

	CLZ_ASSERT(
		r_textures.raw_data[index],
		"stb could not load texture: " + filePath.string()
	);

	r_textures.imageSize[index] =
		r_textures.width[index] * r_textures.height[index] * STBI_rgb_alpha;

	if (!clz::renderer::createImage(
		    r_textures.image[index],
		    filePath,
		    r_textures.width[index],
		    r_textures.height[index],
		    VK_FORMAT_R8G8B8A8_SRGB,
		    VK_IMAGE_TILING_OPTIMAL,
		    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		    0
	    ))
	{
		clz::log::error("Could not create texture: " + filePath.string());
		clz::CLZ_ASSERT(false, "could not initialize all textures");
	}

	setHandleName(
		reinterpret_cast<uint64_t>(r_textures.image[index]),
		VK_OBJECT_TYPE_IMAGE,
		filePath.c_str()
	);

	return r_numRegisteredTextures++;
}

TextureID
registerTexture(const std::byte* data, const size_t size, const std::string& textureName)
{
	CLZ_ASSERT(data, "texture data of: " + textureName + " is invalid");

	r_textures.image.resize(r_textures.image.size() + 1);
	r_textures.imageView.resize(r_textures.image.size() + 1);
	r_textures.imageSize.resize(r_textures.imageSize.size() + 1);
	r_textures.offset.resize(r_textures.offset.size() + 1);
	r_textures.width.resize(r_textures.image.size() + 1);
	r_textures.height.resize(r_textures.image.size() + 1);
	r_textures.numChannels.resize(r_textures.image.size() + 1);
	r_textures.raw_data.resize(r_textures.image.size() + 1);
	const uint32_t index = r_numRegisteredTextures;

	r_textures.raw_data[index] = nullptr;
	r_textures.raw_data[index] = stbi_load_from_memory(
		reinterpret_cast<const stbi_uc*>(data),
		static_cast<int>(size),
		&r_textures.width[index],
		&r_textures.height[index],
		&r_textures.numChannels[index],
		STBI_rgb_alpha
	);
	CLZ_ASSERT(r_textures.raw_data[index], "stb could not load texture: " + textureName);

	r_textures.imageSize[index] = static_cast<VkDeviceSize>(size);

	if (!createImage(
		    r_textures.image[index],
		    textureName,
		    r_textures.width[index],
		    r_textures.height[index],
		    VK_FORMAT_R8G8B8A8_SRGB,
		    VK_IMAGE_TILING_OPTIMAL,
		    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		    0
	    ))
	{
		clz::log::error("vulkan could not create texture: " + textureName);
		clz::CLZ_ASSERT(false, "could not initialize all textures");
	}

	setHandleName(
		reinterpret_cast<uint64_t>(r_textures.image[index]),
		VK_OBJECT_TYPE_IMAGE,
		textureName.c_str()
	);
	return r_numRegisteredTextures++;
}

bool createTextures()
{
	if (r_numRegisteredTextures == 0)
	{
		clz::log::warn("No textures were registered");
		return true;
	}

	// Create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkDeviceSize stagingBufferSize = 0;
	for (uint32_t i = 0; i < r_numRegisteredTextures; ++i)
	{
		stagingBufferSize += r_textures.imageSize[i];
	}

	if (!createBuffer(
		    stagingBuffer,
		    stagingBufferMemory,
		    "Texture staging buffer",
		    stagingBufferSize,
		    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	    ))
	{
		clz::log::error("could not create texture staging buffer");
		return false;
	}

	void* data;
	vkMapMemory(
		r_deviceContext.device,
		stagingBufferMemory,
		0,
		stagingBufferSize,
		0,
		&data
	);
	VkDeviceSize totalStagingBufferOffset = 0;
	for (uint32_t i = 0; i < r_numRegisteredTextures; ++i)
	{
		memcpy(static_cast<std::byte*>(data) + totalStagingBufferOffset,
		       r_textures.raw_data[i],
		       r_textures.imageSize[i]);
		totalStagingBufferOffset += r_textures.imageSize[i];
		stbi_image_free(r_textures.raw_data[i]);
	}
	vkUnmapMemory(r_deviceContext.device, stagingBufferMemory);

	// Creating device local memory
	uint32_t memoryTypeIndexBits = std::numeric_limits<uint32_t>::max();
	VkDeviceSize totalDeviceLocalTextureMemoryRequired = 0;
	for (uint32_t i = 0; i < r_numRegisteredTextures; ++i)
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(
			clz::renderer::r_deviceContext.device,
			r_textures.image[i],
			&memRequirements
		);

		r_textures.offset[i] = nextImageOffset(
			totalDeviceLocalTextureMemoryRequired,
			memRequirements.alignment
		);
		totalDeviceLocalTextureMemoryRequired =
			r_textures.offset[i] + memRequirements.size;

		memoryTypeIndexBits &= memRequirements.memoryTypeBits;
	}
	clz::CLZ_ASSERT(memoryTypeIndexBits, "Da fuq?? No common memory type index found");

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = totalDeviceLocalTextureMemoryRequired;
	memAllocInfo.memoryTypeIndex =
		findMemoryType(memoryTypeIndexBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(
		    clz::renderer::r_deviceContext.device,
		    &memAllocInfo,
		    nullptr,
		    &r_textureDeviceMemory
	    ) != VK_SUCCESS)
	{
		clz::log::error("vulkan could not create texture memory");
		return false;
	}

	for (uint32_t i = 0; i < r_numRegisteredTextures; ++i)
	{
		vkBindImageMemory(
			clz::renderer::r_deviceContext.device,
			r_textures.image[i],
			r_textureDeviceMemory,
			r_textures.offset[i]
		);
	}

	// Finally create the image views now that memory has been allocated
	for (uint32_t i = 0; i < r_numRegisteredTextures; ++i)
	{
		if (!createImageView(
			    r_textures.imageView[i],
			    "entity texture Image view",
			    r_textures.image[i],
			    VK_FORMAT_R8G8B8A8_SRGB,
			    VK_IMAGE_ASPECT_COLOR_BIT
		    ))
		{
			clz::log::error("Could not create image view for textures");
			clz::CLZ_ASSERT(false, "could not initialize all textures");
			return false;
		}
	}

	VkCommandBuffer commandBuffer = startSingleTimeCommand();
	VkDeviceSize totalDeviceLocalOffset = 0;
	for (uint32_t i = 0; i < r_numRegisteredTextures; ++i)
	{
		// transition - 1
		clz::renderer::transition_image_layout(
			r_textures.image[i],
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			commandBuffer
		);

		// Copying buffer to image
		VkBufferImageCopy region = {};
		if (i == 0)
		{
			region.bufferOffset = 0;
			totalDeviceLocalOffset += r_textures.imageSize[i];
		}
		else
		{
			region.bufferOffset = totalDeviceLocalOffset;
			totalDeviceLocalOffset += r_textures.imageSize[i];
		}
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {
			static_cast<uint32_t>(r_textures.width[i]),
			static_cast<uint32_t>(r_textures.height[i]),
			1
		};
		vkCmdCopyBufferToImage(
			commandBuffer,
			stagingBuffer,
			r_textures.image[i],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		// Transition - 2
		clz::renderer::transition_image_layout(
			r_textures.image[i],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			commandBuffer
		);
	}

	submitSingleTimeCommand(commandBuffer);

	vkDestroyBuffer(clz::renderer::r_deviceContext.device, stagingBuffer, nullptr);
	vkFreeMemory(clz::renderer::r_deviceContext.device, stagingBufferMemory, nullptr);

	// Create Sampler
	createSampler();

	return true;
}

bool createSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = clz::renderer::r_gpuInfo.maxAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(
		    clz::renderer::r_deviceContext.device,
		    &samplerInfo,
		    nullptr,
		    &r_sampler
	    ) != VK_SUCCESS)
	{
		clz::log::error("Failed to create texture sampler");
		return false;
	}

	return true;
}

void destroySampler()
{
	vkDestroySampler(clz::renderer::r_deviceContext.device, r_sampler, nullptr);
}

void destroyTextures()
{
	destroySampler();

	for (uint32_t i = 0; i < r_numRegisteredTextures; i++)
	{
		vkDestroyImageView(
			clz::renderer::r_deviceContext.device,
			r_textures.imageView[i],
			nullptr
		);
		vkDestroyImage(
			clz::renderer::r_deviceContext.device,
			r_textures.image[i],
			nullptr
		);
	}

	vkFreeMemory(clz::renderer::r_deviceContext.device, r_textureDeviceMemory, nullptr);
}

} // namespace clz::renderer
