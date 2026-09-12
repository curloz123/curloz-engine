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
	/// @brief processWindowShortcuts
	static void processWindowShortcuts()
	{

	}



#ifdef CLZ_ENABLE_EDITOR
/// --- EDITOR SHORTCUTS --- ///
	/// @brief Processes editor system shortcuts	
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
	}
#endif



}
