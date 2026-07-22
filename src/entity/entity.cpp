#include "entity/entity.hpp"
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
		clearEntities();
		entityCounter = 0;
	}
}