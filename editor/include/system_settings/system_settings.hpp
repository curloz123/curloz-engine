/**
 * @file system_settings.hpp
 * @author curl0z
 * @brief Shows each subsytem's settings in a seperate window
 */
#pragma once

namespace clz::editor
{
	/// @brief Main caller function
	void showSystemSettings();

	/// @brief Shows renderer system's settings.
	/// Implementation is present in system_settings/renderer.cpp
	void showRenderSystemSettings();
}
