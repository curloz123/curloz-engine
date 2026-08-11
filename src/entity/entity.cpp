#include "entity/entity.hpp"

#include "entity/componentmanager.hpp"
#include "entity/entitymanager.hpp"

namespace clz::ecs
{
	void init()
	{
		entityCounter = 0;
		entities.clear();
		entityName.clear();
	}

	void shutdown()
	{
		deleteAllComponents();
		clearEntities();
		entityCounter = 0;
	}
} // namespace clz::ecs