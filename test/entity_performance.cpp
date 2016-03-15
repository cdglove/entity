// ****************************************************************************
// test/entity_functionality_performance.cpp
//
// Part of the test harness for entity.
// 
// Copyright Chris Glover 2014-2016
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
#include <sstream>
#include <iostream>
#include <daily/timer/timer.h>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

#include "entity/component/detail/get_helper.hpp"
#include "entity/iterator/zip_iterator.hpp"

#if STORE_OPTIONAL
  typedef boost::optional<float> storage_t;
#else
  typedef float storage_t;
#endif

static const std::size_t kDefaultNumEntities = TEST_SIZE;
static const float kTestLength = 10.0f;
static const float kFrameTime = 0.016f;
static       bool kUseCreationQueue = false;
static const float kTestDensity = TEST_DENSITY;

#define ALWAYS_TIME_NODE(name) \
	daily::cpu_timer& name ## _timer = daily::timer_map::get_default().create_node(#name); \
	daily::cpu_timer_scope name ## _auto_timer_scope(name ## _timer)

#define BOOST_TEST_MODULE Performance
#include <boost/test/unit_test.hpp>

#if STORE_OPTIONAL == 0
struct jerk
{
	template<typename T>
	void operator()(T a)
	{
		using std::get;
		using boost::get;
		auto ac = get<0>(a);
		if(ac)
			*ac += (0.001f *  kFrameTime);
	}
};

struct accelerate
{
	template<typename T>
	void operator()(T av)
	{
		using std::get;
		using boost::get;
		auto a = get<0>(av);
		auto v = get<1>(av);
		if(a && v)
			*v += *a * kFrameTime;
	}
};

struct move
{
	template<typename T>
	void operator()(T vp)
	{
		using std::get;
		using boost::get;
		auto v = get<0>(vp);
		auto p = get<1>(vp);
		if(v && p)
			*p += *v * kFrameTime;
	}
};
#else
struct jerk
{
	template<typename T>
	void operator()(T a)
	{
		using std::get;
		using boost::get;
		auto ac = get<0>(a);
		if(ac && *ac)
			**ac += (0.001f *  kFrameTime);
	}
};

struct accelerate
{
	template<typename T>
	void operator()(T av)
	{
		using std::get;
		using boost::get;
		auto a = get<0>(av);
		auto v = get<1>(av);
		if(a && v && *a && *v)
			**v += **a * kFrameTime;
	}
};

struct move
{
	template<typename T>
	void operator()(T vp)
	{
		using std::get;
		using boost::get;
		auto v = get<0>(vp);
		auto p = get<1>(vp);
		if(v && p && *v && *p)
			**p += **v * kFrameTime;
	}
};
#endif // OPTIONAL

BOOST_AUTO_TEST_CASE( library_entity )
{
	int argc = boost::unit_test::framework::master_test_suite().argc;
	char** argv = boost::unit_test::framework::master_test_suite().argv;

	int num_entities = kDefaultNumEntities;
	std::stringstream ins;
	for(int i = 1; i < argc; ++i)
	{
		if(strcmp(argv[i], "--num_entities"))
		{
			if(i < argc-1)
			{
				ins.str(argv[i+1]);
				ins >> num_entities;
				if(ins.bad())
					throw std::runtime_error(std::string("Failed to set num_entities from ") + argv[i+1]);
			}
		}
	}

	ALWAYS_TIME_NODE(Total);
	ALWAYS_TIME_NODE(Instantiation);

	entity::entity_pool entities;


#if TEST_DENSE_POOLS
	typedef entity::component::dense_pool<storage_t> position_pool_type;
	typedef entity::component::dense_pool<storage_t> velocity_pool_type;
	typedef entity::component::dense_pool<storage_t> accel_pool_type;
#elif TEST_SPARSE_POOLS
	typedef entity::component::sparse_pool<storage_t> position_pool_type;
	typedef entity::component::sparse_pool<storage_t> velocity_pool_type;
	typedef entity::component::sparse_pool<storage_t> accel_pool_type;
#elif TEST_SATURATED_POOLS
	assert(kTestDensity == 1.f || STORE_OPTIONAL);
	typedef entity::component::saturated_pool<storage_t> position_pool_type;
	typedef entity::component::saturated_pool<storage_t> velocity_pool_type;
	typedef entity::component::saturated_pool<storage_t> accel_pool_type;
#elif TEST_MIXED_POOLS
	typedef entity::component::sparse_pool<storage_t> position_pool_type;
	typedef entity::component::dense_pool<storage_t> velocity_pool_type;
	typedef entity::component::sparse_pool<storage_t> accel_pool_type;
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

	std::clog << "Created Pools" << std::endl;

	std::vector<entity::shared_entity> shuffled_entitys;

	if(false && kTestDensity < 1.f)
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
				for (int i = 0; i < num_entities; ++i)
				{
					shuffled_entitys.push_back(entities.create_shared());
				}

				std::random_device rd;
				std::mt19937 g(rd());
				std::uniform_int_distribution<> dis(0,1);

				for(int i = 0; i < num_entities; ++i)
				{
					if(dis(g))
					{
						std::swap(
							shuffled_entitys[i],
							shuffled_entitys[num_entities-i-1]
						);
					}
				}

				// std::shuffle(
				// 	shuffled_entitys.begin(),
				// 	shuffled_entitys.end(),
				// 	g
				// );

				std::size_t actual_size_to_use = std::size_t(kTestDensity * num_entities);
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

				for (int i = 0; i < num_entities; ++i)
				{
					entities.create();
				}
			}
			
			ALWAYS_TIME_NODE(Create_Components);

			for(auto&& e : entities)
			{
				*position_pool.create(e, 0.f) = 0.f;
				*velocity_pool.create(e, 0.f) = 0.f;
				*accel_pool.create(e, 0.f) = 9.8f;
			}
		}
	}

	std::clog << "Created Components\n"
				 "Simulating " << num_entities << " entities..." << std::endl;
	;

	// Simulate over some seconds using a fixed step.
	{
		ALWAYS_TIME_NODE(Simulation);

		float time_remaining = kTestLength;
		while(time_remaining > 0)
		{
		#if USE_RAW_LOOPS
		
			float* a = &*accel_pool.get(entity::make_entity(0));
			//#pragma loop(no_vector)
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				// Add a little to accel each frame.
				a[i] += 0.001f * kFrameTime;
			}

			float* v = &*velocity_pool.get(entity::make_entity(0));
			//#pragma loop(no_vector)
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				// Compute new velocity.
				v[i] += a[i] * kFrameTime;
			}

			float* p = &*position_pool.get(entity::make_entity(0));
			//#pragma loop(no_vector)
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				// Compute new position.
				p[i] += v[i] * kFrameTime;
			}

		#elif USE_INDEXED_LOOPS && STORE_OPTIONAL
		
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = accel_pool.get(entity::make_entity(i));
				// Add a little to accel each frame.
				if(accel && *accel)
					**accel += 0.001f * kFrameTime;
			}

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = accel_pool.get(entity::make_entity(i));
				auto velocity = velocity_pool.get(entity::make_entity(i));
				// Compute new velocity.
				if(accel && velocity && *accel && *velocity)
					**velocity += **accel * kFrameTime;
			}

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto velocity = velocity_pool.get(entity::make_entity(i));
				auto position = position_pool.get(entity::make_entity(i));
				// Compute new position.
				if(velocity && position && *velocity && *position)
					**position += **velocity * kFrameTime;
			}

		#elif USE_INDEXED_LOOPS && !STORE_OPTIONAL
		
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = accel_pool.get(entity::make_entity(i));
				// Add a little to accel each frame.
				if(accel)
					*accel += 0.001f * kFrameTime;
			}

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = accel_pool.get(entity::make_entity(i));
				auto velocity = velocity_pool.get(entity::make_entity(i));
				// Compute new velocity.
				if(accel && velocity)
					*velocity += *accel * kFrameTime;
			}

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto velocity = velocity_pool.get(entity::make_entity(i));
				auto position = position_pool.get(entity::make_entity(i));
				// Compute new position.
				if(velocity && position)
					*position += *velocity * kFrameTime;
			}

		#elif USE_INDEXED_OPTIONAL_ITERATORS

			auto a_begin = accel_pool.optional_begin();
			auto a_end = accel_pool.optional_end();
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				a_begin.set_target(entity::make_entity(i));
				if(*a_begin)
					**a_begin += (0.001f * kFrameTime);
			}

			a_begin = accel_pool.optional_begin();
			auto v_begin = velocity_pool.optional_begin();
			auto v_end = velocity_pool.optional_end();
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				a_begin.set_target(entity::make_entity(i));
				v_begin.set_target(entity::make_entity(i));
				if(*a_begin && *v_begin)
				{
					**v_begin += **a_begin * kFrameTime;
				}
			}

			v_begin = velocity_pool.optional_begin();
			auto p_begin = position_pool.optional_begin();
			auto p_end = position_pool.optional_end();
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				v_begin.set_target(entity::make_entity(i));
				p_begin.set_target(entity::make_entity(i));
				if(*v_begin && *p_begin)
				{
					**p_begin += **v_begin * kFrameTime;
				}
			}

		#elif USE_GET_HELPER

			auto a_getter = entity::component::detail::make_get_helper(accel_pool);
			auto v_getter = entity::component::detail::make_get_helper(velocity_pool);
			auto p_getter = entity::component::detail::make_get_helper(position_pool);

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = a_getter.get(entity::make_entity(i));
				// Add a little to accel each frame.
				if(accel)
					*accel += 0.001f * kFrameTime;
			}

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = a_getter.get(entity::make_entity(i));
				auto velocity = v_getter.get(entity::make_entity(i));
				// Compute new velocity.
				if(accel && velocity)
					*velocity += *accel * kFrameTime;
			}

			float* p = &*position_pool.get(entity::make_entity(0));
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto velocity = v_getter.get(entity::make_entity(i));
				auto position = p_getter.get(entity::make_entity(i));
				// Compute new position.
				if(velocity && position)
					*position += *velocity * kFrameTime;
			}
		
		#elif USE_ZIP_ITERATOR

			std::for_each(
				entity::iterator::make_zip_iterator(entities.begin(), accel_pool),
				entity::iterator::make_zip_iterator(entities.end(), accel_pool),
				jerk()
			);

			std::for_each(
				entity::iterator::make_zip_iterator(entities.begin(), accel_pool, velocity_pool),
				entity::iterator::make_zip_iterator(entities.end(), accel_pool, velocity_pool),
				accelerate()
			);

			std::for_each(
				entity::iterator::make_zip_iterator(entities.begin(), velocity_pool, position_pool),
				entity::iterator::make_zip_iterator(entities.end(), velocity_pool, position_pool),
				move()
			);

		#elif USE_RANGE

			auto ar = entity::range::combine(entities, accel_pool);
			std::for_each(ar.begin(), ar.end(), jerk());

			auto avr = entity::range::combine(entities, accel_pool, velocity_pool);
			std::for_each(avr.begin(), avr.end(), accelerate());

			auto vpr = entity::range::combine(entities, velocity_pool, position_pool);
			std::for_each(vpr.begin(), vpr.end(), move());

		#elif USE_OPTIONAL_ITERATORS

			auto a_begin = boost::make_zip_iterator(
				boost::make_tuple(accel_pool.optional_begin()));
			auto a_end = boost::make_zip_iterator(
				boost::make_tuple(accel_pool.optional_end()));

			std::for_each(
				a_begin,
				a_end,
				jerk()
			);

			auto av_begin = boost::make_zip_iterator(
				boost::make_tuple(accel_pool.optional_begin(), velocity_pool.optional_begin()));
			auto av_end = boost::make_zip_iterator(
				boost::make_tuple(accel_pool.optional_end(), velocity_pool.optional_end()));

			std::for_each(
				av_begin,
				av_end,
				accelerate()
			);

			auto vp_begin = boost::make_zip_iterator(
				boost::make_tuple(velocity_pool.optional_begin(), position_pool.optional_begin()));
			auto vp_end = boost::make_zip_iterator(
				boost::make_tuple(velocity_pool.optional_end(), position_pool.optional_end()));

			std::for_each(
				vp_begin,
				vp_end,
				move()
			);

		#else
		
			//auto a_range = make_entity_range(entities, zip(accel_pool));
			//std::transform(
			//	a_range.begin(),
			//	a_range.end(),
			//	a_range.begin(),
			//	[](float a)
			//	{
			//		// Add a little to accel each frame.
			//		return a + 0.01f * kFrameTime;
			//	}
			//);

			//auto v_range = make_entity_range(entities, zip(velocity_pool));
			//auto av_range = make_entity_range(entities, zip(accel_pool, velocity_pool));

			//std::transform(
			//	av_range.begin(),
			//	av_range.end(),
			//	v_range.begin(),
			//	[](float a)
			//	{
			//		// Add a little to accel each frame.
			//		return a + 0.01f * kFrameTime;
			//	}
			//);
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
	if(!daily::timer_map::get_default().empty())
	{
		std::cout << "---------- Report -----------\n";
		daily::timer_map::get_default().gather_report(
			std::ostream_iterator<daily::timer_map::result_type>(std::cout, "\n")
		);
	}
	std::cout.flush();
}
