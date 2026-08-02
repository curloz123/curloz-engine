/**
 * @file inspector.hpp
 * @author curl0z
 * @brief Inspector window's functions
 * Inspector window shows all components
 * of current selected entity
 *
 * Also Saves a snapshot upon every change made
 */

#pragma once

#include <vulkan/vulkan.h>

namespace clz::editor
{
/**
 * @brief Renders the Inspector panel for the currently selected entity.
 */
void showInspector(VkCommandBuffer commandBuffer);

/// @brief Identifies which window is in current context right now.
/// Mainly used to check which window to inspect on.
/// State is handled by the window themselves
enum class CurrentFocusedWindow
{
	/// @brief Main scene window
	Scene,

	/// @brief Body shapes editor window
	/// Present in inspector
	BodyEditor
};

/// @brief Gizmo Transform helper
enum class TransformType
{
	TRANSLATE,
	ROTATE,
	SCALE,
};
inline TransformType ActiveTransform = TransformType::TRANSLATE;
} // namespace clz::editor