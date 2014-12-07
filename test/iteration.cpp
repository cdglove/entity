#include "entity/saturated_component_pool.h"
#include "entity/dense_component_pool.h"
#include "entity/sparse_component_pool.h"
#include "entity/entity_pool.h"
#include "entity/entity.h"
#include "entity/component/tie.h"
#include <algorithm>
#include <boost/fusion/include/at_c.hpp>

#define BOOST_TEST_MODULE Iteration
#include <boost/test/unit_test.hpp>

static int const kNumEntities = 1024;

BOOST_AUTO_TEST_CASE( entity_iteration )
{
	entity::entity_pool entities;
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}

	std::for_each(
		entities.begin(),
		entities.end(),
		[](entity::entity const&){}
	);
}

BOOST_AUTO_TEST_CASE( saturated_iteration )
{
	entity::entity_pool entities;
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}

	entity::saturated_component_pool<float> pool(entities);
	std::for_each(
		pool.begin(),
		pool.end(),
		[](float&){}
	);
}

BOOST_AUTO_TEST_CASE( dense_iteration )
{
	entity::entity_pool entities;
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}

	entity::dense_component_pool<float> pool(entities);
	std::for_each(
		pool.begin(),
		pool.end(),
		[](float&){}
	);
}

BOOST_AUTO_TEST_CASE( sparse_iteration )
{
	entity::entity_pool entities;
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}

	entity::sparse_component_pool<float> pool(entities);
	std::for_each(
		pool.begin(),
		pool.end(),
		[](float&){}
	);
}

BOOST_AUTO_TEST_CASE( tied_iteration )
{
	entity::entity_pool entities;
	for(int i = 0; i < kNumEntities; ++i)
	{
		entities.create();
	}
	typedef entity::sparse_component_pool<float> position_pool_type;
	typedef entity::dense_component_pool<float> velocity_pool_type;
	typedef entity::saturated_component_pool<float> accel_pool_type;

	position_pool_type position_pool(entities);
	velocity_pool_type velocity_pool(entities);
	accel_pool_type accel_pool(entities);

	auto b = entity::begin(entities, entity::tie(position_pool, velocity_pool, accel_pool));
	auto e = entity::end(entities, entity::tie(position_pool, velocity_pool, accel_pool));

	if(b != e)
	{
		float& p = *boost::fusion::at_c<0>(*b);
		(void)p;
		float& v = *boost::fusion::at_c<1>(*b);
		(void)v;
		float& a = *boost::fusion::at_c<2>(*b);
		(void)a;
	}
}

BOOST_AUTO_TEST_CASE( list_iteration )
{
	entity::entity_pool entities;
	std::vector<entity::entity> ids;
	for(int i = 0; i < kNumEntities; ++i)
	{
		ids.push_back(entities.create());
	}

	typedef entity::sparse_component_pool<float> position_pool_type;
	typedef entity::dense_component_pool<float> velocity_pool_type;
	typedef entity::saturated_component_pool<float> accel_pool_type;

	position_pool_type position_pool(entities);
	velocity_pool_type velocity_pool(entities);
	accel_pool_type accel_pool(entities);

	auto b = entity::begin(ids, entity::tie(position_pool, velocity_pool, accel_pool));
	auto e = entity::end(ids, entity::tie(position_pool, velocity_pool, accel_pool));

	if(b != e)
	{
		float& p = *boost::fusion::at_c<0>(*b);
		(void)p;
		float& v = *boost::fusion::at_c<1>(*b);
		(void)v;
		float& a = *boost::fusion::at_c<2>(*b);
		(void)a;
	}
}


