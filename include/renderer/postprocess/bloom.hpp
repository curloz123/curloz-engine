/**
 * @file bloom.hpp
 * @author curl0z
 * @brief Defines the Vulkan Bloom post-processing effect structures, resources, and interface.
 *
 * This file contains the declarations for the bloom effect, including image resources,
 * configuration parameters, and the main functions to create, destroy, and apply the
 * bloom pass using a downsample/upsample mip-chain approach.
 */

#pragma once
#include <vulkan/vulkan.h>
#include <array>

namespace clz::renderer::post_process
{
        /**
         * @struct bloomImage
         * @brief Encapsulates a Vulkan image and its associated resources for the bloom pass.
         */
        struct bloomImage
        {
                VkImage image;         ///< The Vulkan image object.
                VkImageView imageView; ///< The view of the image for rendering/sampling.
                VkDeviceMemory memory; ///< The device memory allocated for the image.
                VkExtent2D extent;     ///< The dimensions (width and height) of the image.
        };

        /// @brief The pixel format used for bloom images (HDR RGBA 16-bit float).
        inline constexpr VkFormat BLOOM_IMAGE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;

        /// @brief The number of mip levels used in the bloom downsample/upsample chain.
        inline uint8_t constexpr NUM_BLOOM_MIPS = 5;

        /// @brief Array of images representing the downsampled mip chain for the bloom effect.
        inline std::array<bloomImage, NUM_BLOOM_MIPS> bloomMips;

        /// @brief The final bloomed image that will be combined with the main scene.
        inline bloomImage bloomedImage;

        /// @brief Sampler used for sampling the bloom images during the upsample pass.
        inline VkSampler bloomSampler;

        /**
         * @enum BloomProcessBits
         * @brief Bitmask flags passed to the bloom shader via push constants to control the pass type.
         */
        enum BloomProcessBits : uint32_t
        {
                DOWNSAMPLE = 1 << 0, ///< Indicates the shader should perform a downsample pass.
                UPSAMPLE   = 1 << 1, ///< Indicates the shader should perform an upsample pass.
                DISABLE    = 1 << 2, ///< Indicates the bloom effect is disabled (passthrough/clear).
        };

        /**
         * @brief Initializes and allocates all Vulkan resources required for the bloom post-process.
         * @return true if creation was successful, false otherwise.
         */
        bool createBloomProcess();

        /**
         * @brief Destroys and frees all Vulkan resources associated with the bloom post-process.
	 */
        void destroyBloomProcess();

        /**
         * @brief Records the Vulkan commands to execute the bloom post-process passes.
         * @param commandBuffer The command buffer to record the rendering commands into.
	 *
	 * @note Is explained better in the cpp
         */
        void applyBloomProcess(VkCommandBuffer commandBuffer);

        /// @brief Global flag to enable or disable the bloom effect.
        inline bool Bloom = true;

        /// @brief Global multiplier for the bloom intensity.
        inline float bloomStrength = 0.05f;

        /// @brief Global radius for the bloom filter kernel.
        inline float filterRadius = 0.005;

        /**
         * @brief Disables the bloom effect.
         */
        inline void disableBloom()
        {
                Bloom = false;
        }

        /**
         * @brief Enables the bloom effect.
         */
        inline void enableBloom()
        {
                Bloom = true;
        }

        /**
         * @brief Sets the bloom strength multiplier.
         * @param strength The new bloom strength value.
         */
        inline void setBloomStrength(const float strength)
        {
                bloomStrength = strength;
        }

        /**
         * @brief Gets the current bloom strength multiplier.
         * @return The current bloom strength value.
         */
        inline float getBloomStrength()
        {
                return bloomStrength;
        }

        /**
         * @brief Sets the bloom filter radius.
         * @param radius The new filter radius value.
         */
        inline void setFilterRadius(const float radius)
        {
                filterRadius = radius;
        }

        /**
         * @brief Gets the current bloom filter radius.
         * @return The current filter radius value.
         */
        inline float getFilterRadius()
        {
                return filterRadius;
        }
}
