#include "entity/component/dense_pool.hpp"
#include "entity/component/sparse_pool.hpp"
#include "entity/component/saturated_pool.hpp"
#include "entity/entity_pool.hpp"
#include "entity/entity.hpp"

int main()
{
	entity::entity_pool entities;
	entity::component::saturated_pool<float> sat_pool(entities);
	entity::component::dense_pool<float> dense_pool(entities);
	entity::component::sparse_pool<float> sparse_pool(entities);

	entities.create();

	entity::component::saturated_pool<std::unique_ptr<float>> mo_sat_pool(entities);
	entity::component::dense_pool<std::unique_ptr<float>> mo_dense_pool(entities);
	entity::component::sparse_pool<std::unique_ptr<float>> mo_sparse_pool(entities);

	return 0;
}