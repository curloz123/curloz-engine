/**
 * @file devicecontext.hpp
 * @author curl0z
 * @brief Contains all the functions for
 * initializing the device contexts.
 * ie the vulkan device handles.
 *
 * @note All the contexts are available in variables.hpp
 */
#pragma once

namespace clz::renderer
{
/**
 * @brief Initializes the device context.
 * ie all the device handles.
 * Can view all of them in context.hpp
 */
bool initDeviceContext();

/**
 * @brief Initializes instance
 * @return bool if succeeded
 * and if not, returns false and prints clz::error
 * @note Must be the first handle to be created
 */
bool createInstance();

/**
 * @brief Creates debug messenger
 * @return bool if succeeded
 * and if not, returns false and prints clz::error
 * @note Must be the called after instance has been created
 */
bool createDebugMessenger();

/**
 * @brief Creates window surface
 * @return bool if succeeded
 * and if not, returns false and prints clz::error
 * @note Must be the called after instance has been created
 */
bool createSurface();

/**
 * @brief Selects physical device ie GPU
 * based on whether it supports vulkan API 1.3
 * and has dynamic rendering support.
 *
 * @return bool if succeeded
 * and if not, returns false and prints clz::error
 * @note Selects the best GPU if multiple GPU's
 * are present
 */
bool selectPhysicalDevice();

void extractGPUInfo();

/**
 * @brief Created logical device handle
 * @return void if succeeded or logs the error
 * and returns the error message as string
 * @note Must be the called after
 * physical device has been selected
 */
bool createLogicalDevice();
} // namespace clz::renderer

namespace clz::renderer
{
/**
 * @brief Destroys the device context.
 * @note Must destroy instance first,
 * then surface, then the device
 */
void destroyDeviceContext();

/**
 * @brief Destroys the instance handle
 * @note must be last instance to be destroyed
 */
void destroyInstance();

/**
 * @brief Destroys the debug messenger
 * @note must be called before destroying the instance
 */
void destroyDebugMessenger();

/**
 * @brief Destroys window surface
 */
void destroySurface();

/**
 * @brief Destroys the logical device handle
 */
void destroyDevice();
} // namespace clz::renderer
