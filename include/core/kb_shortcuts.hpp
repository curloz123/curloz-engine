/**
 * @file kb_shortcuts.hpp
 * @author curl0z
 * @brief This file contains all the shorctuts
 * for the whole PROGRAM. Editor, game, window, etc..
 * whatever. All has TO GO HERE
 * @note Editor def still has to be guarded.
 */

#pragma once

namespace clz
{
	/**
	 * @brief Main shortcut processing function.
	 * Has to be called every loop.
	 * Should be present in updateCoreSystems function.
	 */
	void processShortcuts();
}


