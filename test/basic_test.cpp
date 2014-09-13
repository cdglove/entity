#include "entity/dense_component_pool.h"
#include "entity/sparse_component_pool.h"
#include "entity/algorithm/for_each.h"
#include "entity/algorithm/for_all.h"

#include <random>
#include <iostream>

#include "../../cgutil/timer/timer.h"

static const int kNumEntities = 1024 * 128.f;
static const float kTestLength = 0.01f;
static const float kFrameTime = 0.016f;

void time_section(cgutil::timer& t, char const* name)
{
	float elapsed = t.elapsed();
	std::cout << name << " took " << elapsed << " seconds." << std::endl;
	t.reset();
}

int main()
{
	cgutil::timer t;
	{
		entity::entity_pool entities(kNumEntities);

		entity::sparse_component_pool<float> position_pool(entities);
		entity::sparse_component_pool<float> velocity_pool(entities);
		entity::sparse_component_pool<float> accel_pool(entities);

		time_section(t, "Instantiation");

		std::vector<entity::entity> shuffled_entitys;
		for(int i = 0; i < kNumEntities; ++i)
		{
			shuffled_entitys.push_back(entities.create());
		}

		// Mix things up to make it more realistic
		{
			std::random_device rd;
	    	std::mt19937 g(rd());

	    	// Shuffle in 1k chunks or it takes too long.
	    	for(std::size_t i = 0; (i+64*1024) < kNumEntities; i+=64*1024)
	    	{
	 		   	std::shuffle(shuffled_entitys.begin() + i, shuffled_entitys.begin() + i + 64*1024, g);
	    	}

	     	for(std::size_t i = 0; (i+1024) < kNumEntities; i+=1024)
	    	{
	 		   	std::rotate(shuffled_entitys.begin() + i, shuffled_entitys.begin() + i + 1024, shuffled_entitys.end());
	    	}

	    	for(auto e = shuffled_entitys.begin(); e != shuffled_entitys.end(); ++e)
	    	{
	    		entities.destroy(*e);
	    	}

		}
		time_section(t, "Shuffle");

		for(int i = 0; i < kNumEntities; ++i)
		{
			auto e = entities.create();
			auto p = position_pool.create(e);
			auto v = velocity_pool.create(e);
			auto a = accel_pool.create(e);

			*p = 0.f;
			*v = 0.f;
			*a = 9.8f; 
		}

		time_section(t, "Initialization");

		// Simulate over some seconds using a fixed step.
		float time_remaining = kTestLength;
		while(time_remaining > 0)
		{
			entity::for_each(entities, accel_pool, velocity_pool, [](float a, float& v)
			{
				// Compute new velocity.
				v += (a/2.f) * (kTestLength * kTestLength);
			});

			entity::for_each(entities, velocity_pool, position_pool, [](float v, float& p)
			{
				// Compute new position.
				p += v * kFrameTime;
			});

			time_remaining -= kFrameTime;
		}

		time_section(t, "Simulation");
		

		// for(auto e = begin(entities), n = end(entities); e != n; ++e)
		// {
		// 	accel_pool.destroy(*e);
		// 	velocity_pool.destroy(*e);
		// 	position_pool.destroy(*e);
		// 	entities.destroy(*e);
		// }
	}

	time_section(t, "Cleanup");

	return 0;
}