// ****************************************************************************
// test/entity_functionality_performance.cpp
//
// Part of the test harness for entity.
// 
// Copyright Chris Glover 2014
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#define DAILY_INSTRUMENTATION_USE_BOOST_TIMER 1
#define DAILY_ENABLE_INSTRUMENTATION 0

#include "entity/dense_component_pool.hpp"
#include "entity/sparse_component_pool.hpp"
#include "entity/saturated_component_pool.hpp"
#include "entity/algorithm/for_each.hpp"
#include "entity/algorithm/for_all.hpp"
#include "entity/algorithm/simd/sse/for_each.hpp"
#include "entity/component_creation_queue.hpp"
#include "entity/component_destruction_queue.hpp"
#include "entity/component/tie.hpp"
#include "performance_common.hpp"
#include <random>
#include <iostream>
#include <daily/timer/timer.h>

#if ENTITY_SUPPORT_AVX
#  include "entity/algorithm/simd/avx/for_each.hpp"
#endif

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
	typedef entity::dense_component_pool<float> position_pool_type;
	typedef entity::dense_component_pool<float> velocity_pool_type;
	typedef entity::dense_component_pool<float> accel_pool_type;
#elif TEST_SPARSE_POOLS
	typedef entity::sparse_component_pool<float> position_pool_type;
	typedef entity::sparse_component_pool<float> velocity_pool_type;
	typedef entity::sparse_component_pool<float> accel_pool_type;
#elif TEST_SATURATED_POOLS
	assert(kTestDensity == 1.f);
	typedef entity::saturated_component_pool<float> position_pool_type;
	typedef entity::saturated_component_pool<float> velocity_pool_type;
	typedef entity::saturated_component_pool<float> accel_pool_type;
#elif TEST_MIXED_POOLS
	typedef entity::sparse_component_pool<float> position_pool_type;
	typedef entity::dense_component_pool<float> velocity_pool_type;
	typedef entity::sparse_component_pool<float> accel_pool_type;
#endif

	position_pool_type position_pool(entities);
	velocity_pool_type velocity_pool(entities);
	accel_pool_type accel_pool(entities);

	entity::component_creation_queue<
		position_pool_type
	> position_creation_queue(position_pool);

	entity::component_creation_queue<
		velocity_pool_type
	> velocity_creation_queue(velocity_pool);

	entity::component_creation_queue<
		accel_pool_type
	> accel_creation_queue(accel_pool);

	entity::component_destruction_queue<
		position_pool_type
	> position_destruction_queue(position_pool);

	entity::component_destruction_queue<
		velocity_pool_type
	> velocity_destruction_queue(velocity_pool);

	entity::component_destruction_queue<
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
		// The foloowing code, even when not active is causing visual c++
		// to generate a very slow executable.  I have no ida why at this tine.
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

				std::shuffle(
					shuffled_entitys.begin(),
					shuffled_entitys.end(),
					g
				);

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
		#if USE_SSE
			__m128 increment = _mm_set1_ps(0.001f);
			entity::simd::sse::for_each(entities, entity::tie(accel_pool), [&increment](__m128& a)
			{
				DAILY_AUTO_INSTRUMENT_NODE(Simulation_Accel);
				// Add a little to accel each frame.
				a = _mm_add_ps(increment, a);
			});

			// Compute new velocity.
			__m128 divisor = _mm_set1_ps(2.f);
			__m128 frame_time_sq = _mm_set1_ps(kFrameTime * kFrameTime);
			entity::simd::sse::for_each(entities, entity::tie(accel_pool, velocity_pool), [&divisor,&frame_time_sq](__m128 const& a, __m128& v)
			{
				DAILY_AUTO_INSTRUMENT_NODE(Simulation_Velocity);
				v = _mm_add_ps(
						_mm_mul_ps(
							_mm_div_ps(
								a, divisor
							),
						    frame_time_sq
						),
						v
					);
			});

			__m128 frame_time = _mm_set1_ps(kFrameTime);
			entity::simd::sse::for_each(entities, entity::tie(velocity_pool, position_pool), [&frame_time](__m128 const& v, __m128& p)
			{
				DAILY_AUTO_INSTRUMENT_NODE(Simulation_Position);
				// Compute new position.
				p = _mm_add_ps(
						_mm_mul_ps(v, frame_time),
						p
					);
			});
		#elif USE_AVX && ENTITY_SUPPORT_AVX
			__m256 increment = _mm256_set1_ps(0.001f);
			entity::simd::avx::for_each(entities, entity::tie(accel_pool), [&increment](__m256& a)
			{
				// Add a little to accel each frame.
				a = _mm256_add_ps(increment, a);
			});

			// Compute new velocity.
			__m256 divisor = _mm256_set1_ps(2.f);
			__m256 frame_time_sq = _mm256_set1_ps(kFrameTime * kFrameTime);
			entity::simd::avx::for_each(entities, entity::tie(accel_pool, velocity_pool), [&divisor,&frame_time_sq](__m256 a, __m256& v)
			{
				// Compute new velocity.
				v = _mm256_add_ps(
						_mm256_mul_ps(
							_mm256_div_ps(
								a, divisor
							),
						    frame_time_sq
						),
						v
					);
			});

			__m256 frame_time = _mm256_set1_ps(kFrameTime);
			entity::simd::avx::for_each(entities, entity::tie(velocity_pool, position_pool), [&frame_time](__m256 v, __m256& p)
			{
				// Compute new position.
				// Compute new position.
				p = _mm256_add_ps(
						_mm256_mul_ps(v, frame_time),
						p
					);
			});
		#elif USE_RAW_LOOPS
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
				v[i] += (a[i]/2.f) * (kFrameTime * kFrameTime);
			}

			float* __restrict p = position_pool.get(entity::make_entity(0));
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				// Compute new position.
				p[i] += v[i] * kFrameTime;
			}
		#else
			entity::for_each(entities, entity::tie(accel_pool), [](float& a)
			{
				// Add a little to accel each frame.
				a += 0.001f;
			});

			entity::for_each(entities, entity::tie(accel_pool, velocity_pool), [](float a, float& v)
			{
				// Compute new velocity.
				v += (a/2.f) * (kFrameTime * kFrameTime);
			});

			entity::for_each(entities, entity::tie(velocity_pool, position_pool), [](float v, float& p)
			{
				// Compute new position.
				p += v * kFrameTime;
			});
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
		daily::timer_map::get_default().report(std::cout);
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
	daily::timer_map::get_default().report(std::cout);
	std::cout.flush();
}
