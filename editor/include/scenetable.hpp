/**
 * @file editor.hpp
 * @author curl0z
 *
 * @brief Displays everything in scene's biodiversity
 */

#pragma once

#include "entity/entitymanager.hpp"

namespace clz::editor
{
/// @brief In player window, current selected entity
inline std::optional<ecs::entity> currentSelectedEntity;

/// @brief Renders the Entities tab — a selectable list of all scene entities.
void showSceneTab();
} // namespace clz::editor