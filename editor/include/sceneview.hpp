#pragma once

#include "offscreen/offscreentarget.hpp"

namespace clz::editor
{
	/// @brief The main viewport's offscreen target
	inline OffscreenTarget mainViewportImage{};

	/// @brief Draws the mainViewportImage
	void drawSceneView();
}