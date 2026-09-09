/**
 * @file post_tonemap.hpp
 * @author curl0z
 * @brief Defines the post-tonemap post-processing effect structures, resources, and interface.
 *
 * This file contains the declarations for the post-tonemap effect, including image resources,
 * configuration parameters (vignette, chromatic aberration), and the main functions to create,
 * destroy, and apply the post-tonemap pass.
 */

#pragma once

#include "math/vec2.hpp"
#include <vulkan/vulkan.h>
#include <array>

namespace clz::renderer::post_process
{
	/// @brief chromatic aberration 
	inline float chromaticAberrationStrength = 0.0f;

	struct ChromaticAbberationEdgeFade
	{
		float near = 0.35f;
		float far = 0.5f;

		bool operator!=(const ChromaticAbberationEdgeFade& other)
		{
			return (other.near != near) || (other.far != far);
		}
	};
	/// @brief Chromatic aberration's edge fade values
	inline ChromaticAbberationEdgeFade caEdgeFade = {
		.near = 0.35f, 
		.far = 0.5f
	};

	/// @brief Vignette value
	/// index 0 is near, 1 is far
	inline std::array<float, 2> vignette = {0.3f, 0.7f};

}
namespace clz::renderer::post_process
{
        /// @brief The pixel format used for the post-tonemap image (8-bit sRGB).
        inline constexpr VkFormat POST_TONEMAP_IMAGE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;

        /// @brief The Vulkan image object for the post-tonemap pass.
        inline VkImage postTonemapImage;

        /// @brief The view of the post-tonemap image.
        inline VkImageView postTonemapImageView;

        /// @brief The sampler used for sampling the post-tonemap image.
        inline VkSampler postTonemapSampler;

        /// @brief The device memory allocated for the post-tonemap image.
        inline VkDeviceMemory postTonemapMemory;

        /**
         * @brief Initializes and allocates all Vulkan resources required for the post-tonemap pass.
         * @return true if creation was successful, false otherwise.
         */
        bool createPostTonemapProcess();

        /**
         * @brief Destroys and frees all Vulkan resources associated with the post-tonemap pass.
         */
        void destroyPostTonemapProcess();

        /**
         * @brief Runs the post-tonemap pass, writing into postTonemapImage.
         * @param commandBuffer The command buffer to record the rendering commands into.
         * @note Leaves postTonemapImage in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
         * ready to be copied/blitted into the swapchain or editor viewport image.
         */
        void applyPostTonemapProcess(VkCommandBuffer commandBuffer);

        /**
         * @enum PostTonemapEffects
         * @brief Bitmask flags passed to the post-tonemap shader via push constants to control enabled effects.
         */
        enum PostTonemapEffects : uint32_t
        {
                Vignette              = 1 << 0, ///< Enables the vignette effect.
                ChromaticAbberation   = 1 << 1, ///< Enables the chromatic aberration effect.
        };

        /// @brief Global flag to enable or disable the vignette effect.
        inline bool enableVignette = false;

        /**
         * @brief Checks if the vignette effect is enabled.
         * @return true if the vignette effect is enabled, false otherwise.
         */
        inline bool isVignetteEnabled()
        {
                return enableVignette;
        }

        /**
         * @brief Toggles the vignette effect on or off.
         * @param enable True to enable the vignette, false to disable it.
         */
        inline void toggleVignette(const bool enable)
        {
                enableVignette = enable;
        }

        /**
         * @brief Gets the current vignette start and end values.
         * @return A vec2 containing the start (x) and end (y) values.
         * @note Returns default values (0.3f, 0.7f) and logs a warning if values are not set.
         */
        inline math::vec2 getVignette()
	{
		return math::vec2(vignette[0], vignette[1]);
        }

        /**
         * @brief Sets the vignette start and end values.
         * @param start The start radius of the vignette.
         * @param end The end radius of the vignette.
         */
        inline void setVignette(float start, float end)
        {
                vignette[0] = start;
                vignette[1] = end;
        }

        /**
         * @brief Sets the vignette start and end values using a vec2.
         * @param v A vec2 where x is the start radius and y is the end radius.
         */
        inline void setVignette(const math::vec2& v)
        {
                vignette[0] = v.x;
                vignette[1] = v.y;
        }

        /// @brief Global flag to enable or disable the chromatic aberration effect.
        inline bool enableChromaticAberration = false;

        /**
         * @brief Checks if the chromatic aberration effect is enabled.
         * @return true if the chromatic aberration effect is enabled, false otherwise.
         */
        inline bool isChromaticAberrationEnabled()
        {
                return enableChromaticAberration;
        }

        /**
         * @brief Toggles the chromatic aberration effect on or off.
         * @param enable True to enable the effect, false to disable it.
         */
        inline void toggleChromaticAberration(const bool enable)
        {
                enableChromaticAberration = enable;
        }

        /**
         * @brief Gets the current chromatic aberration strength.
         * @return The current strength value.
         */
        inline float getChromaticAberrationStrength()
        {
		return chromaticAberrationStrength;
        }

	inline ChromaticAbberationEdgeFade getCAEdgeFade()
	{
		return caEdgeFade;
	}

        /**
         * @brief Sets the chromatic aberration strength.
         * @param value The new strength value for the chromatic aberration effect.
         */
        inline void setChromaticAberrationStrength(float value)
        {
                chromaticAberrationStrength = value;
        }

	inline void setCAEdgeFade(ChromaticAbberationEdgeFade values)
	{
		caEdgeFade = values;
	}
}
