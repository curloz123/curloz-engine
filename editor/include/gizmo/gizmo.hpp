/**
 * @file gizmo.hpp
 * @brief Defines a simple rectangle structure used for viewport-based positioning.
 *
 * This structure is used to pass viewport coordinates to the transform gizmo
 * drawing functions.
 */

#pragma once

#include <cstdint>

namespace clz::editor
{
	/**
	 * @brief A rectangle defined by its top‑left corner and dimensions.
	 *
	 * Used to specify the area in screen coordinates where the ImGuizmo
	 * manipulation should be rendered.
	 */
	struct Rect2D
	{
		uint32_t x;      /**< X coordinate of the top‑left corner. */
		uint32_t y;      /**< Y coordinate of the top‑left corner. */
		uint32_t width;  /**< Width of the rectangle. */
		uint32_t height; /**< Height of the rectangle. */
	};
}