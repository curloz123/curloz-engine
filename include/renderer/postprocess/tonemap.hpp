/**
 * @file tonemap.hpp
 * @brief Defines the Vulkan Tonemap post-processing effect structures, resources, and interface.
 *
 * This file contains the declarations for the tonemap effect, including image resources,
 * configuration parameters (exposure), and the main functions to create, destroy, and apply the
 * tonemap pass.
 */

#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include "core/logs.hpp"

namespace clz::renderer::post_process
{
        /// @brief The pixel format used for the tonemap image (8-bit sRGB).
        inline constexpr VkFormat TONEMAP_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;

        /// @brief The Vulkan image object for the tonemap pass.
        inline VkImage tonemapImage;

        /// @brief The view of the tonemap image.
        inline VkImageView tonemapImageView;

        /// @brief The sampler used for sampling the tonemap image.
        inline VkSampler tonemapSampler;

        /// @brief The device memory allocated for the tonemap image.
        inline VkDeviceMemory tonemapMemory;

        /**
         * @brief Initializes and allocates all Vulkan resources required for the tonemap pass.
         * @return true if creation was successful, false otherwise.
         */
        bool createTonemapProcess();

        /**
         * @brief Destroys and frees all Vulkan resources associated with the tonemap pass.
         */
        void destroyTonemapProcess();

        /**
         * @brief Records the Vulkan commands to execute the tonemap pass.
         * @param commandBuffer The command buffer to record the rendering commands into.
         */
        void applyTonemapProcess(VkCommandBuffer commandBuffer);

        /// @brief Global exposure value for the tonemap effect.
        inline std::optional<float> exposure;

        /**
         * @brief Sets the exposure value for the tonemap effect.
         * @param newExposure The new exposure value.
         */
        inline void setExposure(const float newExposure)
        {
                exposure = newExposure;
        }

        /**
         * @brief Gets the current exposure value.
         * @return The current exposure value.
         * @note Returns a default value (1.0f) and logs a warning if the value is not set.
         */
        inline float getExposure()
        {
                if (exposure.has_value()) [[likely]]
                {
                        return exposure.value();
                }
                clz::log::warn("Post process's 'exposure' queried, before it was even set");
                return 1.0f;
        }
} // namespace clz::renderer::post_process
