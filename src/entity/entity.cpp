/**
 * @file entity.cpp
 * @author curl0z
 * @brief Entity subsystem public header implementation file
 * Defines initialization and shutdown function
 */
#include "entity/entity.hpp"
#include "entity/componentmanager.hpp"
#include "entity/entitymanager.hpp"

namespace clz::ecs
{
	/// @copydoc init
	void init()
	{
		entityCounter = 0;
		entities.clear();
		entityName.clear();
	}

	/// @copydoc shutdown
	void shutdown()
	{
		deleteAllComponents();
		clearEntities();
		entityCounter = 0;
	}
} // namespace clz::ecs
