#include "entity/dense_component_pool.h"
#include "entity/sparse_component_pool.h"
#include "entity/algorithm/for_each.h"
#include "entity/algorithm/for_all.h"
#include "entity/component_pool_creation_queue.h"
#include "entity/component_pool_destruction_queue.h"

#include <random>
#include <iostream>

#include <cgutil/timer/instrument.h>

static const int kNumEntities = 1024 * 2048;
static const float kTestLength = 1.0f;
static const float kFrameTime = 0.016f;
static const bool kUseCreationQueue = true;
static const bool kUseDestructionQueue = true;

int main()
{
	DECLARE_INSTRUMENT_NODE(Instantiation);
	START_INSTRUMENT_NODE(Instantiation);

	entity::entity_pool entities(kNumEntities);

	typedef entity::sparse_component_pool<float> pool_type;

	pool_type position_pool(entities);
	pool_type velocity_pool(entities);
	pool_type accel_pool(entities);

	entity::component_pool_creation_queue<
		pool_type
	> position_creation_queue(position_pool);

	entity::component_pool_creation_queue<
		pool_type
	> velocity_creation_queue(velocity_pool);

	entity::component_pool_creation_queue<
		pool_type
	> accel_creation_queue(accel_pool);

	entity::component_pool_destruction_queue<
		pool_type
	> position_destruction_queue(position_pool);

	entity::component_pool_destruction_queue<
		pool_type
	> velocity_destruction_queue(velocity_pool);

	entity::component_pool_destruction_queue<
		pool_type
	> accel_destruction_queue(accel_pool);

	STOP_INSTRUMENT_NODE(Instantiation);

	// Mix things up to make it more realistic
	if(true)
	{
		AUTO_INSTRUMENT_NODE(Instantiation);

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

	// Create entities and components.
	{
		AUTO_INSTRUMENT_NODE(CreateEntities);

		std::vector<entity::entity> shuffled_entitys;
		shuffled_entitys.reserve(entities.size());
		for(int i = 0; i < kNumEntities; ++i)
		{
			shuffled_entitys.push_back(entities.create());
		}

		if(kUseCreationQueue)
		{
			AUTO_INSTRUMENT_NODE(QueueCreation);

			for(auto i = shuffled_entitys.begin(); i != shuffled_entitys.end(); ++i)
			{
				auto e = *i;			
				position_creation_queue.push(e, 0.f);
				velocity_creation_queue.push(e, 0.f);
				accel_creation_queue.push(e, 9.8f);
			}


			{ AUTO_INSTRUMENT_NODE(FlushCreation);
			
				position_creation_queue.flush();
				velocity_creation_queue.flush();
				accel_creation_queue.flush();
			}

		}
		else
		{
			AUTO_INSTRUMENT_NODE(ComponentCreation);

			for(auto i = shuffled_entitys.begin(); i != shuffled_entitys.end(); ++i)
			{
				auto e = *i;			
				position_pool.create(e, 0.f);
				velocity_pool.create(e, 0.f);
				accel_pool.create(e, 9.8f);
			}
		}
	}

	// Simulate over some seconds using a fixed step.
	{
		AUTO_INSTRUMENT_NODE(Simulation);

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
	}


    std::cout << "Positions: " << *position_pool.get(entity::entity(0)) << std::endl;
    std::cout << "Velocities: " << *velocity_pool.get(entity::entity(0)) << std::endl;

	cgutil::timer_map::get_default().report(std::cout);
	cgutil::timer_map::get_default().reset_all();
	std::cout << std::endl;

	if(true)
	{
		AUTO_INSTRUMENT_NODE(Cleanup);

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

    cgutil::timer_map::get_default().report(std::cout);

	return 0;
}
