#include "entity/dense_component_pool.h"
#include "entity/sparse_component_pool.h"
#include "entity/algorithm/for_each.h"
#include "entity/algorithm/for_all.h"
#include "entity/algorithm/simd/sse/for_each.h"
#include "entity/algorithm/simd/avx/for_each.h"
#include "entity/component_pool_creation_queue.h"
#include "entity/component_pool_destruction_queue.h"

#include <random>
#include <iostream>

#include <daily/timer/instrument.h>

#if SIZE_OF_TEST
	static const int kNumEntities = SIZE_OF_TEST;
#else
	static const int kNumEntities = 1024 * 2048;
#endif 

static const float kTestLength = 10.0f;
static const float kFrameTime = 0.016f;
static const bool kUseCreationQueue = true;
static const bool kUseDestructionQueue = true;

int main()
{
	DAILY_DECLARE_INSTRUMENT_NODE(Instantiation);
	DAILY_START_INSTRUMENT_NODE(Instantiation);

	entity::entity_pool entities(kNumEntities);

#if TEST_DENSE_POOLS
	typedef entity::dense_component_pool<float> position_pool_type;
	typedef entity::dense_component_pool<float> velocity_pool_type;
	typedef entity::dense_component_pool<float> accel_pool_type;
#elif TEST_SPARSE_POOLS
	typedef entity::sparse_component_pool<float> position_pool_type;
	typedef entity::sparse_component_pool<float> velocity_pool_type;
	typedef entity::sparse_component_pool<float> accel_pool_type;
#elif TEST_MIXED_POOLS
	typedef entity::sparse_component_pool<float> position_pool_type;
	typedef entity::dense_component_pool<float> velocity_pool_type;
	typedef entity::sparse_component_pool<float> accel_pool_type;
#endif

	using boost::fusion::at_c;

	position_pool_type position_pool(entities);
	velocity_pool_type velocity_pool(entities);
	accel_pool_type accel_pool(entities);

	entity::component_pool_creation_queue<
		position_pool_type
	> position_creation_queue(position_pool);

	entity::component_pool_creation_queue<
		velocity_pool_type
	> velocity_creation_queue(velocity_pool);

	entity::component_pool_creation_queue<
		accel_pool_type
	> accel_creation_queue(accel_pool);

	entity::component_pool_destruction_queue<
		position_pool_type
	> position_destruction_queue(position_pool);

	entity::component_pool_destruction_queue<
		velocity_pool_type
	> velocity_destruction_queue(velocity_pool);

	entity::component_pool_destruction_queue<
		accel_pool_type
	> accel_destruction_queue(accel_pool);

	std::clog << "Created Pools\n";

	DAILY_STOP_INSTRUMENT_NODE(Instantiation);

	// Mix things up to make it more realistic
	if(true)
	{
		DAILY_AUTO_INSTRUMENT_NODE(Instantiation);

		std::vector<entity::entity> shuffled_entitys;
		for(int i = 0; i < kNumEntities; ++i)
		{
			shuffled_entitys.push_back(entities.create());
		}

		std::random_device rd;
    	std::mt19937 g(rd());

    	while(shuffled_entitys.size() > 0)
    	{
    		std::uniform_int_distribution<> dis(0, shuffled_entitys.size()-1);
    		auto e = dis(g);
    		entities.destroy(shuffled_entitys[e]);
    		std::swap(shuffled_entitys[e], shuffled_entitys.back());
    		shuffled_entitys.pop_back();
    	}
	}

	std::clog << "Shuffled Entities\n";

	// Create entities and components.
	{
		DAILY_AUTO_INSTRUMENT_NODE(CreateEntities);

		std::vector<entity::entity> shuffled_entitys;
		shuffled_entitys.reserve(entities.size());
		for(int i = 0; i < kNumEntities; ++i)
		{
			shuffled_entitys.push_back(entities.create());
		}

		if(kUseCreationQueue)
		{
			DAILY_AUTO_INSTRUMENT_NODE(QueueCreation);

			for(auto i = shuffled_entitys.begin(); i != shuffled_entitys.end(); ++i)
			{
				auto e = *i;
				position_creation_queue.push(e, 0.f);
				velocity_creation_queue.push(e, 0.f);
				accel_creation_queue.push(e, 9.8f);
			}


			{ DAILY_AUTO_INSTRUMENT_NODE(FlushCreation);
			
				position_creation_queue.flush();
				velocity_creation_queue.flush();
				accel_creation_queue.flush();
			}

		}
		else
		{
			DAILY_AUTO_INSTRUMENT_NODE(ComponentCreation);

			for(auto i = shuffled_entitys.begin(); i != shuffled_entitys.end(); ++i)
			{
				auto e = *i;
				position_pool.create(e, 0.f);
				velocity_pool.create(e, 0.f);
				accel_pool.create(e, 9.8f);
			}
		}
	}

	std::clog << "Created Components, simulating...";

	// Simulate over some seconds using a fixed step.
	{
		DAILY_AUTO_INSTRUMENT_NODE(Simulation);

		float time_remaining = kTestLength;
		while(time_remaining > 0)
		{
		#if USE_SSE
			__m128 increment = _mm_set1_ps(0.001f);
			entity::simd::sse::for_each(entities, accel_pool, [&increment](__m128& a)
			{
				// Add a little to accel each frame.
				a = _mm_add_ps(increment, a);
			});

			// Compute new velocity.
			__m128 divisor = _mm_set1_ps(2.f);
			__m128 frame_time_sq = _mm_set1_ps(kFrameTime * kFrameTime);
			entity::simd::sse::for_each(entities, accel_pool, velocity_pool, [&divisor,&frame_time_sq](__m128 const& a, __m128& v)
			{
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
			entity::simd::sse::for_each(entities, velocity_pool, position_pool, [&frame_time](__m128 const& v, __m128& p)
			{
				// Compute new position.
				p = _mm_add_ps(_mm_mul_ps(v, frame_time), p);
			});
		#elif USE_AVX && ENTITY_SUPPORT_AVX
			entity::simd::avx::for_each(entities, accel_pool, [](__m256& a)
			{
				// Add a little to accel each frame.
				__m256 increment = _mm256_set1_ps(0.001f);
				a += increment;
			});

			entity::simd::avx::for_each(entities, accel_pool, velocity_pool, [](__m256 a, __m256& v)
			{
				// Compute new velocity.
				__m256 divisor = _mm256_set1_ps(2.f);
				__m256 frame_time_sq = _mm256_set1_ps(kFrameTime * kFrameTime);
				v += (a/divisor) * frame_time_sq;
			});

			entity::simd::avx::for_each(entities, velocity_pool, position_pool, [](__m256 v, __m256& p)
			{
				// Compute new position.
				__m256 frame_time = _mm256_set1_ps(kFrameTime);
				p += v * frame_time;
			});
		#else
			entity::for_each(entities, entity::tie(accel_pool), [](boost::fusion::vector<float*> a)
			{
				// Add a little to accel each frame.
				*at_c<0>(a) += 0.001f;
			});

			entity::for_each(entities, entity::tie(accel_pool, velocity_pool), [](boost::fusion::vector<float*, float*> const& av)
			{
				// Compute new velocity.
				*at_c<1>(av) += (*at_c<0>(av)/2.f) * (kFrameTime * kFrameTime);
			});

			entity::for_each(entities, entity::tie(velocity_pool, position_pool), [](boost::fusion::vector<float*, float*> const& vp)
			{
				// Compute new position.
				*at_c<1>(vp) += *at_c<0>(vp) * kFrameTime;
			});
		#endif

			time_remaining -= kFrameTime;
		}
	}

	std::clog << "done.\n";

    std::clog << "Positions: " << *position_pool.get(entity::entity(0)) << std::endl;
    std::clog << "Velocities: " << *velocity_pool.get(entity::entity(0)) << std::endl;

	daily::timer_map::get_default().report(std::cout);
	daily::timer_map::get_default().reset_all();
	std::cout << std::endl;

	if(true)
	{
		DAILY_AUTO_INSTRUMENT_NODE(Cleanup);

		std::vector<entity::entity> entitys_list;
		entitys_list.reserve(kNumEntities);
		for(auto i = begin(entities); i != end(entities); ++i)
		{
			entitys_list.push_back(*i);
		}

		std::random_device rd;
    	std::mt19937 g(rd());

    	if(kUseDestructionQueue)
    	{
	    	while(entitys_list.size() > 0)
	    	{
	    		std::uniform_int_distribution<> dis(0, entitys_list.size()-1);
	    		auto idx = dis(g);
	    		auto e = entitys_list[idx];
	    		
	    		position_destruction_queue.push(e);
				velocity_destruction_queue.push(e);
				accel_destruction_queue.push(e);

	    		entities.destroy(e);
	    		std::swap(entitys_list[idx], entitys_list.back());
	    		entitys_list.pop_back();
	    	}

	    	position_destruction_queue.flush();
	    	velocity_destruction_queue.flush();
	    	accel_destruction_queue.flush();
    	}
    	else
    	{
	    	while(entitys_list.size() > 0)
	    	{
	    		std::uniform_int_distribution<> dis(0, entitys_list.size()-1);
	    		auto idx = dis(g);
	    		auto e = entitys_list[idx];
	    		
	    		position_pool.destroy(e);
				velocity_pool.destroy(e);
				accel_pool.destroy(e);

	    		entities.destroy(e);
	    		std::swap(entitys_list[idx], entitys_list.back());
	    		entitys_list.pop_back();
	    	}
	    }
    }

    daily::timer_map::get_default().report(std::cout);

	return 0;
}
