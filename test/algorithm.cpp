// ****************************************************************************
// test/algorithm.cpp
//
// Algorithm test harness for entity.
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************

#include "entity/dense_component_pool.hpp"
#include "entity/sparse_component_pool.hpp"
#include "entity/saturated_component_pool.hpp"
#include "entity/entity_pool.hpp"
#include "entity/entity.hpp"
#include "entity/algorithm/for_each.hpp"
#include "entity/algorithm/for_any.hpp"
#include "entity/component/tie.hpp"

#define BOOST_TEST_MODULE Algorithm
#include <boost/test/unit_test.hpp>

struct entities_t
{
	entities_t()
		: sat_pool(entities)
		, dense_pool(entities)
		, sparse_pool(entities)
	{
		sat_pool.auto_create_components(entities, 1.f);
		dense_pool.auto_create_components(entities, 2.f);
		sparse_pool.auto_create_components(entities, 3.f);

		for(int i = 0; i < 1000; ++i)
		{
			entities.create();
		}
	}

	entity::entity_pool entities;
	entity::saturated_component_pool<float> sat_pool;
	entity::dense_component_pool<float> dense_pool;
	entity::sparse_component_pool<float> sparse_pool;
};

BOOST_AUTO_TEST_CASE( algorithm_for_each )
{
	entities_t the_entities;

	entity::for_each(
		the_entities.entities,
		entity::tie(the_entities.sat_pool, the_entities.dense_pool, the_entities.sparse_pool),
		[](float& a, float& b, float &c)
		{
			a += 1.f;
			b += 2.f;
			c += 3.f;
		}
	);

	for(auto&& e : the_entities.entities)
	{
		BOOST_CHECK_EQUAL(*the_entities.sat_pool.get(e), 2.f);
		BOOST_CHECK_EQUAL(*the_entities.dense_pool.get(e), 4.f);
		BOOST_CHECK_EQUAL(*the_entities.sparse_pool.get(e), 6.f);
	}
}

BOOST_AUTO_TEST_CASE( algorithm_for_any )
{
	entities_t the_entities;

	for(auto&& e : the_entities.entities)
	{
		the_entities.dense_pool.destroy(e);
		if(e.index() % 2)
			the_entities.sparse_pool.destroy(e);
	}

	entity::for_any(
		the_entities.entities,
		entity::tie(the_entities.sat_pool, the_entities.dense_pool, the_entities.sparse_pool),
		[](float* a, float* b, float *c)
		{
			if(a)
				*a += 1.f;

			if(b)
				*b += 2.f;

			if(c)
				*c += 3.f;
		}
	);

	for(auto&& e : the_entities.entities)
	{
		float* a = the_entities.sat_pool.get(e);
		float* b = the_entities.dense_pool.get(e);
		float* c = the_entities.sparse_pool.get(e);

		if(a)
			BOOST_CHECK_EQUAL(*a, 2.f);

		if(b)
			BOOST_CHECK_EQUAL(*b, 4.f);

		if(c)
			BOOST_CHECK_EQUAL(*c, 6.f);

	}
}