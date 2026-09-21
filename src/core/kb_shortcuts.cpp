/**
 * @file kb_shortcuts.cpp
 * @author curl0z
 * @brief This file contains all the shorctuts
 * for the whole PROGRAM. Editor, game, window, etc..
 * whatever. All has TO GO HERE
 * @note Editor def still has to be guarded.
 */

#include "core/kb_shortcuts.hpp"
#include "window/inputmanager.hpp"
#include "window/window.hpp"
#include "scene/scene.hpp"

#ifdef CLZ_ENABLE_EDITOR
#include "../../editor/include/gizmo/gizmo.hpp"
#include "../../editor/include/timemachine.hpp"
#include "../../editor/include/timemachine.hpp"
#endif

///< @brief All sub-system local processer function def go here
namespace clz
{
	/// @brief Processes window system shortcuts
	static void processWindowShortcuts();

#ifdef CLZ_ENABLE_EDITOR
	/// @brief Processes editor system shortcuts	
	static void processEditorShortcuts();
#endif

	/// @brief Helper function that returns whether no modifier is pressed
	static inline bool noModiferPressed()
	{
		using namespace clz::window;
		return !(isKeyPressed(input::Key::LeftSuper) ||
			isKeyPressed(input::Key::RightSuper) ||
			isKeyPressed(input::Key::LeftAlt) ||
			isKeyPressed(input::Key::RightAlt) ||
			isKeyPressed(input::Key::LeftShift) ||
			isKeyPressed(input::Key::RightShift) ||
			isKeyPressed(input::Key::LeftControl) ||
			isKeyPressed(input::Key::RightControl));
	}
}

///< @brief All definitions
namespace clz
{
	/// --- MAIN PROCESSOR FUNCTION --- ///
	/// @copydoc processShortcuts
	void processShortcuts()
	{
		processWindowShortcuts();

#ifdef CLZ_ENABLE_EDITOR
		processEditorShortcuts();
#endif

	}

	/// --- WINDOW SHORTCUTS --- ///
	/**
	 * @brief processWindowShortcuts.
	 * Currently Has ->
	 * 1. F11: Make window exclusive fullscreen.
	 */
	static void processWindowShortcuts()
	{
		static bool F11PressLastFrame = false;
		if (window::isKeyPressed(input::Key::F11) && noModiferPressed())
		{
			if (!F11PressLastFrame)
				window::toggleExclusiveFullscreenMode();

			F11PressLastFrame = true;
		}
		else
		{
			F11PressLastFrame = false;
		}
	}



#ifdef CLZ_ENABLE_EDITOR
	/// --- EDITOR SHORTCUTS --- ///
	/**
	 * @brief Processes editor system shortcuts.
	 * Currently Has ->
	 * 1. T: Sets gizmo mode to transform.
	 * 2. R: Sets gizmo mode to rotate.
	 * 3. S: Sets gizmo mode to scale.
	 * 4. Ctrl + S: Saves back to config
	 */
	static void processEditorShortcuts()
	{
		if (window::isKeyPressed(input::Key::T) && noModiferPressed())
		{
			editor::ActiveTransform = editor::TransformType::TRANSLATE;
		}
		if (window::isKeyPressed(input::Key::R) && noModiferPressed())
		{
			editor::ActiveTransform = editor::TransformType::ROTATE;
		}
		if (window::isKeyPressed(input::Key::S) && noModiferPressed())
		{
			editor::ActiveTransform = editor::TransformType::SCALE;
		}

		/// --- Scene saving key-bind --- ///
		static bool CTRL_SPressedLastFrame = false;
		if (window::isKeyPressed(input::Key::LeftControl) &&
			window::isKeyPressed(input::Key::S))
		{
			if (!CTRL_SPressedLastFrame)
			{
				/// --- Don't save every nanosecond key is held --- ///	
				clz::scene::saveScene();
				clz::timemachine::clearSnapshots();
			}
			CTRL_SPressedLastFrame = true;
		}
		else
		{
			CTRL_SPressedLastFrame = false;
		}

		/// --- Undo and Redo --- ///
		static bool undoKeyPressedLastFrame = false;
		static bool redoKeyPressedLastFrame = false;
		if (window::isKeyPressed(input::Key::LeftControl) &&
			window::isKeyPressed(input::Key::Z))
		{
			if (window::isKeyPressed(input::Key::LeftShift))
			{
				if (!redoKeyPressedLastFrame)
					timemachine::performRedo();
				redoKeyPressedLastFrame = true;
			}
			else
			{
				if (!undoKeyPressedLastFrame)
					timemachine::performUndo();
				undoKeyPressedLastFrame = true;
			}
		}
		else
		{
			undoKeyPressedLastFrame = false;
			redoKeyPressedLastFrame = false;
		}
	}
#endif



}
