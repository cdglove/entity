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
#include <numeric>
#include <random>


#define BOOST_TEST_MODULE Iteration
#include <boost/test/unit_test.hpp>


static int const kNumEntities = 5;

// ----------------------------------------------------------------------------
//
template<typename Pool>
void SimpleIteratePool()
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
		[](typename Pool::type& v){}
	);
}

// ----------------------------------------------------------------------------
//
template<typename Pool>
void OptionalSimpleIteratePool()
{
	entity::entity_pool entities;
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}

	Pool pool(entities);
	std::for_each(
		pool.optional_begin(),
		pool.optional_end(),
		[](entity::component::optional<typename Pool::type>){}
	);
}

// ----------------------------------------------------------------------------
//
template<typename Pool>
void AccumulatePool()
{
	entity::entity_pool entities;
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}

	Pool pool(entities);
	std::transform(
		pool.begin(),
		pool.end(),
		pool.begin(),
		[](typename Pool::type)
		{
			return 1;
		}
	);

	typename Pool::type sum = std::accumulate(
		pool.begin(),
		pool.end(),
		0);

	BOOST_TEST_CHECK(sum == kNumEntities);
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
			BOOST_TEST_CHECK(i == 1);
		}
	);

	std::for_each(
		entities.begin(),
		entities.end(),
		[&velocity_pool](entity::entity e)
		{
			int i = *velocity_pool.get(e);
			BOOST_TEST_CHECK(i == 2);
		}
	);

	std::for_each(
		entities.begin(),
		entities.end(),
		[&accel_pool](entity::entity e)
		{
			int i = *accel_pool.get(e);
			BOOST_TEST_CHECK(i == 3);
		}
	);
}

// ----------------------------------------------------------------------------
//
struct add_0_1
{
	template<typename T>
	int operator()(T e)
	{
		int a = entity::component::get<0>(e);
		int b = entity::component::get<1>(e);
		return a + b;
	}
};

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
		add_0_1()
	);

	auto vel_range = entity::make_entity_range(entities, zip(position_pool, velocity_pool));
	std::transform(
		vel_range.begin(),
		vel_range.end(),
		position_pool.begin(),
		add_0_1()
	);

	std::for_each(
		position_pool.begin(),
		position_pool.end(),
		[](int p)
		{
			BOOST_TEST_CHECK(p == 6);
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
	SimpleIteratePool<entity::component::saturated_pool<float>>();
}

BOOST_AUTO_TEST_CASE( dense_iteration )
{
	SimpleIteratePool<entity::component::dense_pool<float>>();
}

BOOST_AUTO_TEST_CASE( sparse_iteration )
{
	SimpleIteratePool<entity::component::sparse_pool<float>>();
}

BOOST_AUTO_TEST_CASE( optional_saturated_iteration )
{
	OptionalSimpleIteratePool<entity::component::saturated_pool<float>>();
}

BOOST_AUTO_TEST_CASE( optional_dense_iteration )
{
	OptionalSimpleIteratePool<entity::component::dense_pool<float>>();
}

BOOST_AUTO_TEST_CASE( optional_sparse_iteration )
{
	OptionalSimpleIteratePool<entity::component::sparse_pool<float>>();
}

BOOST_AUTO_TEST_CASE( saturated_accumulation )
{
	AccumulatePool<entity::component::saturated_pool<int>>();
}

BOOST_AUTO_TEST_CASE( dense_accumulation )
{
	AccumulatePool<entity::component::dense_pool<int>>();
}

BOOST_AUTO_TEST_CASE( sparse_accumulation )
{
	AccumulatePool<entity::component::sparse_pool<int>>();
}

BOOST_AUTO_TEST_CASE( optional_saturated_accumulation )
{
	entity::entity_pool entities;
	entity::component::saturated_pool<int> pool(entities, 1);
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}

	int sum = std::accumulate(
		pool.begin(),
		pool.end(),
		0);

	BOOST_TEST_CHECK(sum == kNumEntities);
}

BOOST_AUTO_TEST_CASE( optional_dense_accumulation )
{
	entity::entity_pool entities;
	entity::component::saturated_pool<int> pool(entities);
	int expected_sum = 0;
	for(int i = 0; i < kNumEntities; ++i)
	{
		auto e = entities.create();
		if(i % 2)
		{
			expected_sum += i;
			*pool.get(e) = i;
		}
	}

	int sum = std::accumulate(
		pool.optional_begin(),
		pool.optional_end(),
		0,
		[](int current, entity::component::required<int> v)
		{
			if(v)
				return *v + current;
			else
				return current;
		}
	);

	BOOST_TEST_CHECK(sum == expected_sum);
}

BOOST_AUTO_TEST_CASE( optional_sparse_accumulation )
{
	entity::entity_pool entities;
	entity::component::saturated_pool<int> pool(entities);
	int expected_sum = 0;
	for(int i = 0; i < kNumEntities; ++i)
	{
		auto e = entities.create();
		if(!(i % 2))
		{
			expected_sum += i;
			*pool.get(e) = i;
		}
	}

	int sum = std::accumulate(
		pool.optional_begin(),
		pool.optional_end(),
		0,
		[](int current, entity::component::required<int> v)
		{
			if(v)
				return *v + current;
			else
				return current;
		}
	);

	BOOST_TEST_CHECK(sum == expected_sum);
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


