/**
 * @file gizmo.hpp
 * @brief Defines a simple rectangle structure used for viewport-based positioning.
 *
 * This structure is used to pass viewport coordinates to the transform gizmo
 * drawing functions.
 */

#pragma once

#include <cstdint>
#include "entity/corecomponents.hpp"
#include "math/mat4x4.hpp"
#include <tuple>

namespace clz::editor
{
	/// @brief Gizmo Transform helper
	enum class TransformType
	{
		TRANSLATE,
		ROTATE,
		SCALE
	};

	/// @brief current Active Transform to show
	inline TransformType ActiveTransform = TransformType::TRANSLATE;

	/**
	 * @brief A rectangle defined by its top‑left corner and dimensions.
	 *
	 * Used to specify the area in screen coordinates where the ImGuizmo
	 * manipulation should be rendered.
	 */
	struct Rect2D
	{
		uint32_t x;	 /**< X coordinate of the top‑left corner. */
		uint32_t y;	 /**< Y coordinate of the top‑left corner. */
		uint32_t width;	 /**< Width of the rectangle. */
		uint32_t height; /**< Height of the rectangle. */
	};

	/**
	 * @brief Shows gizmo for transformation of only entity's transform component
	 * @param viewport 2D rectangular extents of the image where gizmo is to be shown
	 * @param projection Camera's projection matrix
	 * @param view camera's view matrix
	 * @param transform entity's current transform component(the quat one)
	 * @return Tuple consisting of ->
	 * - transform entity component
	 * - editor component
	 * - bool which signifies whether gizmo was used this frame or not.
	 * @note projection must not be passed as a const reference
	 * @note On no change, will simply return old values.
	 */
	std::tuple<ecs::TransformComponent, ecs::EditorTransformComponent, bool> gizmoTransform(
		const Rect2D& viewport,
		clz::math::mat4& projection,
		const clz::math::mat4& view,
		const ecs::TransformComponent transform, 
		const bool uniformScale = false
	);

	/**
	 * @brief Shows gizmo for transformation of only entity's transform component
	 * @param viewport 2D rectangular extents of the image where gizmo is to be shown
	 * @param projection Camera's projection matrix
	 * @param view camera's view matrix
	 * @param transform entity's current transform component (the euler one)
	 * @return Tuple consisting of ->
	 * - transform entity component
	 * - editor component
	 * - bool which signifies whether gizmo was used this frame or not.
	 * @note projection must not be passed as a const reference
	 * @note On no change, will simply return old values.
	 */
	std::tuple<ecs::TransformComponent, ecs::EditorTransformComponent, bool> gizmoTransform(
		const Rect2D& viewport,
		clz::math::mat4& projection,
		const clz::math::mat4& view,
		const ecs::EditorTransformComponent eTransform, 
		const bool uniformScale = false
	);

} // namespace clz::editor
