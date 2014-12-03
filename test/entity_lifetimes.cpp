#include "entity/saturated_component_pool.h"
#include "entity/entity_pool.h"
#include "entity/entity.h"

#define BOOST_TEST_MODULE Lifetimes
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( no_ownership )
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