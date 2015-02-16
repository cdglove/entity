// ****************************************************************************
// test/signals.cpp
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
#include "entity/dense_component_pool.hpp"
#include "entity/sparse_component_pool.hpp"
#include "entity/saturated_component_pool.hpp"
#include "entity/entity_pool.hpp"
#include "entity/entity.hpp"
#include <algorithm>

#define BOOST_TEST_MODULE Signals
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( manual_component_creation )
{
	entity::entity_pool entities;
	entity::saturated_component_pool<float> sat_pool(entities);
	entity::dense_component_pool<float> dense_pool(entities);
	entity::sparse_component_pool<float> sparse_pool(entities);

	entities.create();
	entities.create();
	entities.create();

	BOOST_CHECK_EQUAL(entities.size(), 3);
	BOOST_CHECK_EQUAL(sat_pool.size(), 0);
	BOOST_CHECK_EQUAL(dense_pool.size(), 0);
	BOOST_CHECK_EQUAL(sparse_pool.size(), 0);
}

BOOST_AUTO_TEST_CASE( auto_component_creation )
{
	entity::entity_pool entities;
	entity::saturated_component_pool<float> sat_pool(entities);
	entity::dense_component_pool<float> dense_pool(entities);
	entity::sparse_component_pool<float> sparse_pool(entities);

	sat_pool.auto_create_components(entities, 1.f);
	dense_pool.auto_create_components(entities, 2.f);
	sparse_pool.auto_create_components(entities, 3.f);

	std::vector<entity::entity> entity_list;
	entity_list.push_back(entities.create());
	entity_list.push_back(entities.create());
	entity_list.push_back(entities.create());

	BOOST_CHECK_EQUAL(entities.size(), 3);
	BOOST_CHECK_EQUAL(sat_pool.size(), 3);
	BOOST_CHECK_EQUAL(dense_pool.size(), 3);
	BOOST_CHECK_EQUAL(sparse_pool.size(), 3);

	BOOST_CHECK_EQUAL(*sat_pool.get(entity::make_entity(0)), 1.f);
	BOOST_CHECK_EQUAL(*dense_pool.get(entity::make_entity(0)), 2.f);
	BOOST_CHECK_EQUAL(*sparse_pool.get(entity::make_entity(0)), 3.f);

	for(auto&& e : entity_list)
	{
		entities.destroy(e);
	}

	BOOST_CHECK_EQUAL(entities.size(), 0);
	BOOST_CHECK_EQUAL(sat_pool.size(), 0);
	BOOST_CHECK_EQUAL(dense_pool.size(), 0);
	BOOST_CHECK_EQUAL(sparse_pool.size(), 0);
}

BOOST_AUTO_TEST_CASE( component_destruction )
{
	entity::entity_pool entities;
	entity::saturated_component_pool<float> sat_pool(entities);
	entity::dense_component_pool<float> dense_pool(entities);
	entity::sparse_component_pool<float> sparse_pool(entities);

	sat_pool.auto_create_components(entities, 1.f);
	dense_pool.auto_create_components(entities, 2.f);
	sparse_pool.auto_create_components(entities, 3.f);

	std::vector<entity::entity> entity_list;
	entity_list.push_back(entities.create());
	entity_list.push_back(entities.create());
	entity_list.push_back(entities.create());

	for(auto&& e : entity_list)
	{
		entities.destroy(e);
	}

	BOOST_CHECK_EQUAL(entities.size(), 0);
	BOOST_CHECK_EQUAL(sat_pool.size(), 0);
	BOOST_CHECK_EQUAL(dense_pool.size(), 0);
	BOOST_CHECK_EQUAL(sparse_pool.size(), 0);
}