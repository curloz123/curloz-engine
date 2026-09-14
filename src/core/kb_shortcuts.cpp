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
		if (window::isKeyPressed(input::Key::F11))
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
		if (window::isKeyPressed(input::Key::T))
		{
			editor::ActiveTransform = editor::TransformType::TRANSLATE;
		}
		if (window::isKeyPressed(input::Key::R))
		{
			editor::ActiveTransform = editor::TransformType::ROTATE;
		}
		if (window::isKeyPressed(input::Key::S))
		{
			editor::ActiveTransform = editor::TransformType::SCALE;
		}

		static bool CTRL_SPressedLastFrame = false;
		if (window::isKeyPressed(input::Key::LeftControl) &&
			!CTRL_SPressedLastFrame)
		{
			/// This branch is inside to avoid alone S shortcut
			if (window::isKeyPressed(input::Key::S))
			{
				clz::scene::saveScene();
				CTRL_SPressedLastFrame = true;
			}
		}
		else
		{
			CTRL_SPressedLastFrame = false;
		}
	}
#endif



}
