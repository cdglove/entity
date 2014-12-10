#include "entity/saturated_component_pool.h"
#include "entity/dense_component_pool.h"
#include "entity/sparse_component_pool.h"
#include "entity/entity_pool.h"
#include "entity/entity.h"
#include "entity/component/tie.h"
#include <algorithm>
#include <boost/fusion/include/at_c.hpp>
#include <random>

#define BOOST_TEST_MODULE Iteration
#include <boost/test/unit_test.hpp>

static int const kNumEntities = 5;

// ----------------------------------------------------------------------------
//
template<typename Pool>
void IteratePool()
{
	entity::entity_pool entities;
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}

	Pool pool(entities);
	std::for_each(
		pool.begin(),
		pool.end(),
		[](typename Pool::type&){}
	);
}

// ----------------------------------------------------------------------------
//
template<typename EntityList>
void IterateTied(entity::entity_pool& pool, EntityList& entities)
{
	typedef entity::sparse_component_pool<int> position_pool_type;
	typedef entity::dense_component_pool<int> velocity_pool_type;
	typedef entity::saturated_component_pool<int> accel_pool_type;

	position_pool_type position_pool(pool);
	velocity_pool_type velocity_pool(pool);
	accel_pool_type accel_pool(pool);

	auto b = entity::begin(entities, entity::tie(position_pool, velocity_pool, accel_pool));
	auto e = entity::end(entities, entity::tie(position_pool, velocity_pool, accel_pool));

	while(b != e)
	{
		int& p = *boost::fusion::at_c<0>(*b);
		p = 1;
		int& v = *boost::fusion::at_c<1>(*b);
		v = 2;
		int& a = *boost::fusion::at_c<2>(*b);
		a = 3;
		++b;
	}

	std::for_each(
		entities.begin(),
		entities.end(),
		[&position_pool](entity::entity e)
		{
			int i = *position_pool.get(e);
			//while(i != 1) {}
			BOOST_REQUIRE_EQUAL(i, 1);
		}
	);

	std::for_each(
		entities.begin(),
		entities.end(),
		[&velocity_pool](entity::entity e)
		{
			int i = *velocity_pool.get(e);
			//while(i != 2) {}
			BOOST_REQUIRE_EQUAL(i, 2);
		}
	);

	std::for_each(
		entities.begin(),
		entities.end(),
		[&accel_pool](entity::entity e)
		{
			int i = *accel_pool.get(e);
			//while(i != 3) {}
			BOOST_REQUIRE_EQUAL(i, 3);
		}
	);
}

// ----------------------------------------------------------------------------
//
std::unique_ptr<entity::entity_pool> CreateFilledPool()
{
	auto entities = std::unique_ptr<entity::entity_pool>(new entity::entity_pool());
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities->create();
	}

	return entities;
}

BOOST_AUTO_TEST_CASE( entity_iteration )
{
	auto entities = CreateFilledPool();

	std::for_each(
		entities->begin(),
		entities->end(),
		[](entity::entity const&){}
	);
}

BOOST_AUTO_TEST_CASE( saturated_iteration )
{
	IteratePool<entity::saturated_component_pool<float>>();
}

BOOST_AUTO_TEST_CASE( dense_iteration )
{
	IteratePool<entity::dense_component_pool<float>>();
}

BOOST_AUTO_TEST_CASE( sparse_iteration )
{
	IteratePool<entity::sparse_component_pool<float>>();
}

BOOST_AUTO_TEST_CASE( tied_iteration )
{
	auto entities = CreateFilledPool();
	IterateTied(*entities, *entities);
}

BOOST_AUTO_TEST_CASE( list_iteration )
{
	auto entities = CreateFilledPool();
	std::vector<entity::entity> ids;
	for(entity::entity_index_t i = 0; i < kNumEntities; ++i)
	{
		ids.push_back(entity::make_entity(i));
	}

	IterateTied(*entities, ids);
}

BOOST_AUTO_TEST_CASE( partial_list_iteration )
{
	auto entities = CreateFilledPool();
	std::vector<entity::entity> ids;
	for(entity::entity_index_t i = 0; i < kNumEntities; ++i)
	{
		ids.push_back(entity::make_entity(i));
	}

	std::random_device rd;
	std::mt19937 g(rd());

	std::shuffle(ids.begin(), ids.end(), g);
	ids.erase(ids.begin(), ids.begin() + (ids.size() / 2));
	std::sort(ids.begin(), ids.end());
	IterateTied(*entities, ids);
}

//BOOST_AUTO_TEST_CASE( unordered_list_iteration )
//{
//	auto entities = CreateFilledPool();
//	std::vector<entity::entity> ids;
//	for(entity::entity_index_t i = 0; i < kNumEntities; ++i)
//	{
//		ids.push_back(entity::make_entity(i));
//	}
//
//	std::random_device rd;
//	std::mt19937 g(rd());
//
//	std::shuffle(ids.begin(), ids.end(), g);
//	IterateTied(*entities, ids);
//}
//
//BOOST_AUTO_TEST_CASE( unordered_partial_iteration )
//{
//	auto entities = CreateFilledPool();
//	std::vector<entity::entity> ids;
//	for(entity::entity_index_t i = 0; i < kNumEntities; ++i)
//	{
//		ids.push_back(entity::make_entity(i));
//	}
//
//	std::random_device rd;
//	std::mt19937 g(rd());
//
//	std::shuffle(ids.begin(), ids.end(), g);
//	ids.erase(ids.begin(), ids.begin() + (ids.size() / 2));
//	IterateTied(*entities, ids);
//}


