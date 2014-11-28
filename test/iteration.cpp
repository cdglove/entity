#include "entity/saturated_component_pool.h"
#include "entity/entity_pool.h"
#include "entity/entity.h"
#include <algorithm>

static int const kNumEntities = 1024;

int main()
{
	entity::entity_pool entities;
	std::for_each(
		entities.begin(),
		entities.end(),
		[](entity::entity const&){}
	);

	entity::saturated_component_pool<float> sat_pool(entities);
	std::for_each(
		sat_pool.begin(),
		sat_pool.end(),
		[](float&){}
	);
	
	return 0;
}