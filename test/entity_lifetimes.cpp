// ****************************************************************************
// test/entity_lifetimes.cpp
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
#include "entity/entity_pool.hpp"
#include "entity/entity.hpp"

#define BOOST_TEST_MODULE Lifetimes
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( manual_ownership )
{
	entity::entity_pool entities;
	entities.create();
	BOOST_CHECK_EQUAL(entities.size(), 1);
	entities.create();
	BOOST_CHECK_EQUAL(entities.size(), 2);
	entities.destroy(entity::make_entity(0));
	BOOST_CHECK_EQUAL(entities.size(), 1);
	entities.destroy(entity::make_entity(0));
	BOOST_CHECK(entities.empty());
}

BOOST_AUTO_TEST_CASE( unique_ownership )
{
	// Doesnt compile on MSVC < 1900 due
	// to that compiler not correctly generating
	// implicit move operations.
#if _MSC_VER >= 1900
	entity::entity_pool entities;
	entity::unique_entity e = entities.create_unique();
	BOOST_CHECK_EQUAL(entities.size(), 1);
	entity::unique_entity e2 = entities.create_unique();
	BOOST_CHECK_EQUAL(entities.size(), 2);
	e = std::move(e2);
	BOOST_CHECK_EQUAL(entities.size(), 1);
	e2.clear();
	BOOST_CHECK_EQUAL(entities.size(), 1);
	e.clear();
	BOOST_CHECK(entities.empty());
#endif
}

BOOST_AUTO_TEST_CASE( shared_ownership )
{
	entity::entity_pool entities;
	entity::shared_entity e = entities.create_shared();
	BOOST_CHECK_EQUAL(entities.size(), 1);
	entity::shared_entity e2 = entities.create_shared();
	BOOST_CHECK_EQUAL(entities.size(), 2);
	e = e2;
	BOOST_CHECK_EQUAL(entities.size(), 1);
	e.clear();
	BOOST_CHECK_EQUAL(entities.size(), 1);
	e2.clear();
	BOOST_CHECK(entities.empty());
}