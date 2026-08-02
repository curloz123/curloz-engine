/**
 * @file commandcontext.hpp
 * @author curl0z
 *
 * @brief File containing all relevant function to command pools
 * and command buffers
 *
 * @note Must be initialized before creating swapchain
 */
#pragma once

namespace clz::renderer
{
/**
 * @brief Initializes command pool and buffers
 */
bool initCommandContext();

/**
 * @brief Destroys command pool and buffer
 */
void destroyCommandContext();
} // namespace clz::renderer