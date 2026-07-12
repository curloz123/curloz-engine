/**
 * @file rigidbodycomponent.hpp
 * @author curl0z
 * @brief displays model component's data n all
 * in inspector. Internally can change rigid body's
 * data and body's attributes.
 */

#pragma once

#include <vector>
#include "renderer/shapes.hpp"
#include "physics/shape.hpp"
#include <set>

namespace clz::editor
{
	void showRigidBodyHeader();

	void drawBodyEditorOffscreenImage(VkCommandBuffer commandBuffer);
	void presentBodyEditorWindow();

	inline bool anyChanges = false;
	inline std::set<uint32_t> changedShapesIndex;
	inline std::vector<physics::BoxShape> changedShapes;
	inline std::vector<physics::BoxShape> newShapes;
	//inline std::vector<physics::BoxShape> changedShapes;
}