// ****************************************************************************
// test/entity_functionality_performance.cpp
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
//#define DAILY_INSTRUMENTATION_USE_BOOST_TIMER 1
//#define DAILY_ENABLE_INSTRUMENTATION 1

#include "entity/all.hpp"
#include "performance_common.hpp"
#include <daily/timer/instrument.h>
#include <random>
#include <iostream>
#include <daily/timer/timer.h>

static const std::size_t kNumEntities = TEST_SIZE;
static const float kTestLength = 10.0f;
static const float kFrameTime = 0.016f;
static       bool kUseCreationQueue = false;
static const float kTestDensity = TEST_DENSITY;

#define ALWAYS_TIME_NODE(name) \
	daily::cpu_timer& name ## _timer = daily::timer_map::get_default().create_node(#name); \
	daily::cpu_timer_scope name ## _auto_timer_scope(name ## _timer)

#define BOOST_TEST_MODULE Performance
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( library_entity )
{
	ALWAYS_TIME_NODE(Total);
	ALWAYS_TIME_NODE(Instantiation);

	entity::entity_pool entities;

#if TEST_DENSE_POOLS
	typedef entity::component::dense_pool<float> position_pool_type;
	typedef entity::component::dense_pool<float> velocity_pool_type;
	typedef entity::component::dense_pool<float> accel_pool_type;
#elif TEST_SPARSE_POOLS
	typedef entity::component::sparse_pool<float> position_pool_type;
	typedef entity::component::sparse_pool<float> velocity_pool_type;
	typedef entity::component::sparse_pool<float> accel_pool_type;
#elif TEST_SATURATED_POOLS
	assert(kTestDensity == 1.f);
	typedef entity::component::saturated_pool<float> position_pool_type;
	typedef entity::component::saturated_pool<float> velocity_pool_type;
	typedef entity::component::saturated_pool<float> accel_pool_type;
#elif TEST_MIXED_POOLS
	typedef entity::component::sparse_pool<float> position_pool_type;
	typedef entity::component::dense_pool<float> velocity_pool_type;
	typedef entity::component::sparse_pool<float> accel_pool_type;
#endif

	position_pool_type position_pool(entities);
	velocity_pool_type velocity_pool(entities);
	accel_pool_type accel_pool(entities);

	entity::component::creation_queue<
		position_pool_type
	> position_creation_queue(position_pool);

	entity::component::creation_queue<
		velocity_pool_type
	> velocity_creation_queue(velocity_pool);

	entity::component::creation_queue<
		accel_pool_type
	> accel_creation_queue(accel_pool);

	entity::component::destruction_queue<
		position_pool_type
	> position_destruction_queue(position_pool);

	entity::component::destruction_queue<
		velocity_pool_type
	> velocity_destruction_queue(velocity_pool);

	entity::component::destruction_queue<
		accel_pool_type
	> accel_destruction_queue(accel_pool);

	Instantiation_timer.stop();

	std::clog << "Created Pools\n";

	std::vector<entity::shared_entity> shuffled_entitys;

	if(kTestDensity < 1.f)
	{
		kUseCreationQueue = true;
	}
	
	// ------------------------------------------------------------------------
	{
		if(kUseCreationQueue)
		{
			// Create entities and components.
			{
				ALWAYS_TIME_NODE(Create_Entities);

				shuffled_entitys.reserve(entities.size());
				for (int i = 0; i < kNumEntities; ++i)
				{
					shuffled_entitys.push_back(entities.create_shared());
				}

				std::random_device rd;
				std::mt19937 g(rd());
				std::uniform_int_distribution<> dis(0,1);

				for(int i = 0; i < kNumEntities; ++i)
				{
					if(dis(g))
					{
						std::swap(
							shuffled_entitys[i],
							shuffled_entitys[kNumEntities-i-1]
						);
					}
				}

				// std::shuffle(
				// 	shuffled_entitys.begin(),
				// 	shuffled_entitys.end(),
				// 	g
				// );

				std::size_t actual_size_to_use = std::size_t(kTestDensity * kNumEntities);
				shuffled_entitys.resize(actual_size_to_use);
			}

			ALWAYS_TIME_NODE(Create_Components);

			for(auto i = shuffled_entitys.begin(); i != shuffled_entitys.end(); ++i)
			{
				auto e = *i;
				position_creation_queue.push(e, 0.f);
				velocity_creation_queue.push(e, 0.f);
				accel_creation_queue.push(e, 9.8f);
			}

			ALWAYS_TIME_NODE(Flush_Create_Components);
			position_creation_queue.flush();
			velocity_creation_queue.flush();
			accel_creation_queue.flush();
		}
		else
		{
			// ----------------------------------------------------------------
			{
				ALWAYS_TIME_NODE(Create_Entities);

				for (int i = 0; i < kNumEntities; ++i)
				{
					entities.create();
				}
			}
			
			ALWAYS_TIME_NODE(Create_Components);

			for(auto&& e : entities)
			{
				position_pool.create(e, 0.f);
				velocity_pool.create(e, 0.f);
				accel_pool.create(e, 9.8f);
			}
		}
	}

	std::clog << "Created Components\n"
				 "Simulating " << kNumEntities << " entities..." 
	;

	// Simulate over some seconds using a fixed step.
	{
		ALWAYS_TIME_NODE(Simulation);

		float time_remaining = kTestLength;
		while(time_remaining > 0)
		{
		#if USE_RAW_LOOPS
			float* __restrict a = accel_pool.get(entity::make_entity(0));
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				// Add a little to accel each frame.
				a[i] += 0.001f;
			}

			float* __restrict v = velocity_pool.get(entity::make_entity(0));
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				// Compute new velocity.
				v[i] += a[i] * kFrameTime;
			}

			float* __restrict p = position_pool.get(entity::make_entity(0));
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				// Compute new position.
				p[i] += v[i] * kFrameTime;
			}
		#else
			//for(auto i : entity::component::zip(accel_pool))
			//{

			//}
			//entity::for_each(entities, entity::component::tie(accel_pool), [](float& a)
			//{
			//	// Add a little to accel each frame.
			//	a += 0.001f;
			//});

			//entity::for_each(entities, entity::component::tie(accel_pool, velocity_pool), [](float a, float& v)
			//{
			//	// Compute new velocity.
			//	v += (a/2.f) * (kFrameTime * kFrameTime);
			//});

			//entity::for_each(entities, entity::component::tie(velocity_pool, position_pool), [](float v, float& p)
			//{
			//	// Compute new position.
			//	p += v * kFrameTime;
			//});
		#endif
			time_remaining -= kFrameTime;
		}
	}

	
	std::clog << "done." << std::endl;


    std::clog << "Positions: " << *position_pool.begin() << std::endl;
    std::clog << "Velocities: " << *velocity_pool.begin() << std::endl;

	if(!daily::timer_map::get_default().empty())
	{
		std::cout << "---------- Report -----------\n";
		daily::timer_map::get_default().gather_report(
			std::ostream_iterator<daily::timer_map::result_type>(std::cout, "\n")
		);
	}

	daily::timer_map::get_default().reset_all();

	if(!kUseCreationQueue)
	{
		ALWAYS_TIME_NODE(Cleanup);

		while(entities.size() > 0)
		{
			entities.destroy(entity::make_entity(0));
		}
	}
	else
	{
		ALWAYS_TIME_NODE(Cleanup);
		// Make sure entities are cleaed up in reverse, otherwise
		// it takes a long time for sparce pool.
		std::sort(
			shuffled_entitys.begin(),
			shuffled_entitys.end(), 
			std::greater<entity::shared_entity>()
		);
		shuffled_entitys.clear();
	}

	Total_timer.stop();
	daily::timer_map::get_default().gather_report(
		std::ostream_iterator<daily::timer_map::result_type>(std::cout, "\n")
	);
	std::cout.flush();
}
