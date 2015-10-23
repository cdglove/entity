// ****************************************************************************
// test/iteration.cpp
//
// Part of the test harness for entity.
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#include "entity/component/saturated_pool.hpp"
#include "entity/component/dense_pool.hpp"
#include "entity/component/sparse_pool.hpp"
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
	typedef entity::component::sparse_pool<int> position_pool_type;
	typedef entity::component::dense_pool<int> velocity_pool_type;
	typedef entity::component::saturated_pool<int> accel_pool_type;

	position_pool_type position_pool(pool);
	velocity_pool_type velocity_pool(pool);	
	accel_pool_type accel_pool(pool);

	auto range = entity::make_entity_range(entities, zip(position_pool, velocity_pool, accel_pool));
	for(auto i = range.begin(); i != range.end(); ++i)
	{
		int& p = entity::component::get<0>(*i);
		p = 1;
		int& v = entity::component::get<1>(*i);
		v = 2;
		int& a = entity::component::get<2>(*i);
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
template<typename EntityList>	
void TransformTied(entity::entity_pool& pool, EntityList& entities)
{
	typedef entity::component::sparse_pool<int> position_pool_type;
	typedef entity::component::dense_pool<int> velocity_pool_type;
	typedef entity::component::saturated_pool<int> accel_pool_type;

	position_pool_type position_pool(pool);
	velocity_pool_type velocity_pool(pool);	
	accel_pool_type accel_pool(pool);

	auto range = entity::make_entity_range(entities, zip(position_pool, velocity_pool, accel_pool));
	for(auto i = range.begin(); i != range.end(); ++i)
	{
		int& p = entity::component::get<0>(*i);
		p = 1;
		int& v = entity::component::get<1>(*i);
		v = 2;
		int& a = entity::component::get<2>(*i);
		a = 3;
	}

	auto accel_range = entity::make_entity_range(entities, zip(velocity_pool, accel_pool));
	std::transform(
		accel_range.begin(),
		accel_range.end(),
		velocity_pool.begin(),
		[](auto e)
		{
			int v = entity::component::get<0>(e);
			int a = entity::component::get<1>(e);
			return v + a;
		}
	);

	auto vel_range = entity::make_entity_range(entities, zip(position_pool, velocity_pool));
	std::transform(
		vel_range.begin(),
		vel_range.end(),
		position_pool.begin(),
		[](auto e)
		{
			int p = entity::component::get<0>(e);
			int v = entity::component::get<1>(e);
			return v + p;
		}
	);

	std::for_each(
		position_pool.begin(),
		position_pool.end(),
		[](int p)
		{
			BOOST_REQUIRE_EQUAL(p, 6);
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
	IteratePool<entity::component::saturated_pool<float>>();
}

BOOST_AUTO_TEST_CASE( dense_iteration )
{
	IteratePool<entity::component::dense_pool<float>>();
}

BOOST_AUTO_TEST_CASE( sparse_iteration )
{
	IteratePool<entity::component::sparse_pool<float>>();
}

BOOST_AUTO_TEST_CASE( tied_iteration )
{
	auto entities = CreateFilledPool();
	IterateTied(*entities, *entities);
}

BOOST_AUTO_TEST_CASE( tied_transform )
{
	auto entities = CreateFilledPool();
	TransformTied(*entities, *entities);
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
// Unordered currently unsupported.
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


