#pragma once

#include <GLFW/glfw3.h>

namespace clz::input
{
	// Key definitions matching GLFW key codes
	enum class Mouse : int
	{
		// Mouse clicks
		MouseLeft = GLFW_MOUSE_BUTTON_LEFT,
		MouseRight = GLFW_MOUSE_BUTTON_RIGHT,
		MouseMiddle = GLFW_MOUSE_BUTTON_MIDDLE,
	};

	enum class Key : int
	{
		Escape = GLFW_KEY_ESCAPE,
		Space = GLFW_KEY_SPACE,
		Enter = GLFW_KEY_ENTER,
		Tab = GLFW_KEY_TAB,
		Backspace = GLFW_KEY_BACKSPACE,
		LeftControl = GLFW_KEY_LEFT_CONTROL,
		RightControl = GLFW_KEY_RIGHT_CONTROL,
		LeftAlt = GLFW_KEY_LEFT_ALT,
		RightAlt = GLFW_KEY_RIGHT_ALT,
		LeftShift = GLFW_KEY_LEFT_SHIFT,
		RightShift = GLFW_KEY_RIGHT_SHIFT,
		LeftSuper = GLFW_KEY_LEFT_SUPER,
		RightSuper = GLFW_KEY_RIGHT_SUPER,

		// Arrow keys
		Up = GLFW_KEY_UP,
		Down = GLFW_KEY_DOWN,
		Left = GLFW_KEY_LEFT,
		Right = GLFW_KEY_RIGHT,

		// Letters
		A = GLFW_KEY_A,
		B = GLFW_KEY_B,
		C = GLFW_KEY_C,
		D = GLFW_KEY_D,
		E = GLFW_KEY_E,
		F = GLFW_KEY_F,
		G = GLFW_KEY_G,
		H = GLFW_KEY_H,
		I = GLFW_KEY_I,
		J = GLFW_KEY_J,
		K = GLFW_KEY_K,
		L = GLFW_KEY_L,
		M = GLFW_KEY_M,
		N = GLFW_KEY_N,
		O = GLFW_KEY_O,
		P = GLFW_KEY_P,
		Q = GLFW_KEY_Q,
		R = GLFW_KEY_R,
		S = GLFW_KEY_S,
		T = GLFW_KEY_T,
		U = GLFW_KEY_U,
		V = GLFW_KEY_V,
		W = GLFW_KEY_W,
		X = GLFW_KEY_X,
		Y = GLFW_KEY_Y,
		Z = GLFW_KEY_Z,

		// Numbers
		Num0 = GLFW_KEY_0,
		Num1 = GLFW_KEY_1,
		Num2 = GLFW_KEY_2,
		Num3 = GLFW_KEY_3,
		Num4 = GLFW_KEY_4,
		Num5 = GLFW_KEY_5,
		Num6 = GLFW_KEY_6,
		Num7 = GLFW_KEY_7,
		Num8 = GLFW_KEY_8,
		Num9 = GLFW_KEY_9,

		// Function keys
		F1 = GLFW_KEY_F1,
		F2 = GLFW_KEY_F2,
		F3 = GLFW_KEY_F3,
		F4 = GLFW_KEY_F4,
		F5 = GLFW_KEY_F5,
		F6 = GLFW_KEY_F6,
		F7 = GLFW_KEY_F7,
		F8 = GLFW_KEY_F8,
		F9 = GLFW_KEY_F9,
		F10 = GLFW_KEY_F10,
		F11 = GLFW_KEY_F11,
		F12 = GLFW_KEY_F12,
	};
}