#include "entity/component/dense_pool.hpp"
#include "entity/component/sparse_pool.hpp"
#include "entity/component/saturated_pool.hpp"
#include "entity/entity_pool.hpp"
#include "entity/entity.hpp"

static int const kNumEntities = 1024;

int main()
{
	entity::entity_pool entities;
	entity::component::saturated_pool<float> sat_pool(entities);
	entity::component::dense_pool<float> dense_pool(entities);
	entity::component::sparse_pool<float> sparse_pool(entities);
	return 0;
}