#include "entity/saturated_component_pool.hpp"
#include "entity/dense_component_pool.hpp"
#include "entity/sparse_component_pool.hpp"
#include "entity/entity_pool.hpp"
#include "entity/entity.hpp"
#include "entity/component/zip.hpp"
#include "entity/component/tie.hpp"
#include "entity/entity_range.hpp"
#include <algorithm>
#include <boost/fusion/include/at_c.hpp>
#include <random>


#define BOOST_TEST_MODULE Iteration
#include <boost/test/unit_test.hpp>

//#undef BOOST_REQUIRE_EQUAL
//#define BOOST_REQUIRE_EQUAL(a, b) assert(a == b)

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

	auto range = entity::make_entity_range(entities, entity::zip(entity::tie(position_pool, velocity_pool, accel_pool)));
	for(auto i = range.begin(); i != range.end(); ++i)
	{
		int& p = entity::get<0>(*i);
		p = 1;
		int& v = entity::get<1>(*i);
		v = 2;
		int& a = entity::get<2>(*i);
		a = 3;
	}

	std::for_each(
		entities.begin(),
		entities.end(),
		[&position_pool](entity::entity e)
		{
			int i = *position_pool.get(e);
			BOOST_REQUIRE_EQUAL(i, 1);
		}
	);

	std::for_each(
		entities.begin(),
		entities.end(),
		[&velocity_pool](entity::entity e)
		{
			int i = *velocity_pool.get(e);
			BOOST_REQUIRE_EQUAL(i, 2);
		}
	);

	std::for_each(
		entities.begin(),
		entities.end(),
		[&accel_pool](entity::entity e)
		{
			int i = *accel_pool.get(e);
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


