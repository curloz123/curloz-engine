#pragma once

namespace clz::ecs
{

/**
 * @brief Simply disables the entity
 * All the components attached with the entity that has this tag
 * will stop updating
 */
struct DisableTagComponent
{
	DisableTagComponent() = default;
};

/**
 * @brief Tag that specifies whether entity is marked for deletion or not
 * @note For the time being until engine hasn't been shutdown since deletion,
 * this will just disable the entity
 */
struct DeletionTagComponent
{
	DeletionTagComponent() = default;
};

} // namespace clz::ecs
