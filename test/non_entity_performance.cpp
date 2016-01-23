// ****************************************************************************
// test/non_entity_performance.cpp
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
#define DAILY_ENABLE_INSTRUMENTATION 0
#include <iostream>
#include <vector>
#include <algorithm>
#include <daily/timer/instrument.h>
#include "performance_common.hpp"

static const int kNumEntities = TEST_SIZE;
static const float kTestLength = 10.0f;
static const float kFrameTime = 0.016f;

struct PhysicsData
{
	unsigned int id;
	float position;
	float velocity;
	float accel;
#if EXTRA_PADDING
	char pad[EXTRA_PADDING];
#endif
};

template<typename I, typename Fn>
void for_each(I i, I e, Fn f)
{
	#pragma loop( no_vector )
	for(; i != e; ++i)
	{
		f(*i);
	}
}

#define BOOST_TEST_MODULE Performance
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( non_entity )
{
	std::vector<PhysicsData> entities;

	std::clog << "Created Pools\n";

	{	 DAILY_AUTO_INSTRUMENT_NODE(ComponentCreation);

		unsigned int ids = 0;
		for(int i = 0; i < kNumEntities; ++i)
		{
			PhysicsData p;
			p.id = ids++;
			p.accel = 9.8f;
			p.velocity = 0;
			p.position = 0;
			entities.push_back(p);
		}
	}

	std::clog << "Created Components, simulating...";

	// Simulate over some seconds using a fixed step.
	{
		daily::cpu_timer& simulation_node = daily::timer_map::get_default().create_node("Simulation");
		daily::cpu_timer_scope simulation_scope(simulation_node);

		float time_remaining = kTestLength;
		while(time_remaining > 0)
		{
			::for_each(entities.begin(), entities.end(), [](PhysicsData& p)
			{
				// Add a little to accel each frame.
				p.accel += 0.001f;
			});

			::for_each(entities.begin(), entities.end(), [](PhysicsData& p)
			{
				// Compute new velocity.
				p.velocity += (p.accel/2.f) * (kFrameTime * kFrameTime);
			});

			::for_each(entities.begin(), entities.end(), [](PhysicsData& p)
			{
				// Compute new position.
				p.position += p.velocity * kFrameTime;
			});

			time_remaining -= kFrameTime;
		}
	}

	std::clog << "done.\n";

    std::clog << "Positions: " << entities[0].position << std::endl;
    std::clog << "Velocities: " << entities[0].velocity << std::endl;

	if(!daily::timer_map::get_default().empty())
	{
		std::cout << "---------- Report -----------\n";
		daily::timer_map::get_default().gather_report(
			std::ostream_iterator<daily::timer_map::result_type>(std::cout, "\n")
		);
	}

	std::cout.flush();
}
