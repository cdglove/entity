#include "entity/entity_pool.h"
#include "entity/dense_component_pool.h"
#include "entity/sparse_component_pool.h"

#include <iostream>

static const int kNumEntities = 1024 * 2048;
static const float kTestLength = 10.0f;
static const float kFrameTime = 0.016f;
static const bool kUseCreationQueue = true;
static const bool kUseDestructionQueue = true;

int main()
{
	entity::entity_pool entities(kNumEntities);

#if TEST_DENSE_POOLS
	typedef entity::dense_component_pool<float> position_pool_type;
	typedef entity::dense_component_pool<float> velocity_pool_type;
	typedef entity::dense_component_pool<float> accel_pool_type;
#elif TEST_SPARSE_POOLS
	typedef entity::sparse_component_pool<float> position_pool_type;
	typedef entity::sparse_component_pool<float> velocity_pool_type;
	typedef entity::sparse_component_pool<float> accel_pool_type;
#else
	typedef entity::sparse_component_pool<float> position_pool_type;
	typedef entity::dense_component_pool<float> velocity_pool_type;
	typedef entity::sparse_component_pool<float> accel_pool_type;
#endif

	position_pool_type position_pool(entities);
	velocity_pool_type velocity_pool(entities);
	accel_pool_type accel_pool(entities);

	auto b = entity::begin(entities, entity::tie(position_pool, velocity_pool));
	auto e = entity::end(entities, entity::tie(position_pool, velocity_pool));

	if(b == e)
	{

	}

	return 0;
}