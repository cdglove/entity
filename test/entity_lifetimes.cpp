#include "entity/saturated_component_pool.h"
#include "entity/entity_pool.h"
#include "entity/entity.h"

static int const kNumEntities = 1024;

int main()
{
	entity::entity_pool entities(kNumEntities);

	for (entity::entity_index_t i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}

	for (entity::entity_index_t i = 0; i < kNumEntities; ++i)
	{
		entities.destroy(entity::make_entity(i));
	}

	return 0;
}