/**
 * @file entitytransformgizmo.hpp
 * @brief Declaration of the function that draws an interactive transform gizmo.
 *
 * The gizmo allows editing of the selected entity's translation, rotation,
 * and scale directly in the viewport.
 */

#pragma once

#include "gizmo.hpp"

namespace clz::editor
{
/**
 * @brief Draws the transform gizmo for the currently selected entity.
 *
 * This function integrates with ImGuizmo to render a manipulator (translate,
 * rotate, or scale) over the main viewport. It updates the entity's
 * transform components when the user drags the gizmo.
 *
 * @param viewport The screen rectangle (in pixels) that defines the active
 *                 area for the gizmo. Typically matches the main viewport.
 *
 * @par Decision: Active transform mode
 *      The operation is selected via the global `ActiveTransform` variable
 *      (defined elsewhere) to toggle between TRANSLATE, ROTATE, and SCALE.
 *
 * @par Decision: Editor vs. runtime transform separation
 *      The entity has both a runtime `TransformComponent` and an editor‑only
 *      `EditorTransformComponent`. The gizmo manipulates the editor copy,
 *      and then applies the changes to the runtime component (with Euler‑to‑quaternion
 *      conversion). This allows the editor to work with Euler angles while
 *      the runtime uses quaternions.
 */
void drawEntityTransformGizmo(const Rect2D& viewport);
} // namespace clz::editor
