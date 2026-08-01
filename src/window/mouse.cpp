#include "window/mouse.hpp"
#include "window/window_types.hpp"

namespace clz::window
{
/// @copydoc
void cursorCallback(GLFWwindow* window, double xPos, double yPos)
{
	w_cursorPosition = {static_cast<float>(xPos), static_cast<float>(yPos)};
}

/// @copydoc
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	w_scrollOffset = static_cast<float>(yOffset);
}
} // namespace clz::window