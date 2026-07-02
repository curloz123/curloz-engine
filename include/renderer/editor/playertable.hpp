/**
* @file editor.hpp
* @author curl0z
*
* @brief Native editor's variables
*/

#pragma once

#include <optional>
#include "ecs/entity.hpp"

namespace clz::editor
{
	/// @brief Currently selected entity in the entity panel. Empty if nothing is selected.
	inline std::optional<ecs::entity> currentSelectedEntity;

	/// @brief Renders the Entities tab — a selectable list of all scene entities.
	void showEntityTab();
}