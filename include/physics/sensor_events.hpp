#include "entity/entity.hpp"
#include "entity/entitymanager.hpp"

namespace clz::physics
{
	struct SensorEntityPair
	{
		ecs::entity entityA;
		ecs::entity entityB;
	};

	void processSensorEvents();
}
