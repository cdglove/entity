#include "entity/dense_component_pool.h"
#include "entity/sparse_component_pool.h"
#include "entity/saturated_component_pool.h"
#include "entity/entity_pool.h"
#include "entity/entity.h"

static int const kNumEntities = 1024;

int main()
{
	entity::entity_pool entities;
	entity::saturated_component_pool<float> sat_pool(entities);
	entity::dense_component_pool<float> dense_pool(entities);
	entity::sparse_component_pool<float> sparse_pool(entities);
	return 0;
}