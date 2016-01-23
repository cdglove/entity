// ****************************************************************************
// test/manual_entity_performance.cpp
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
#include "performance_common.hpp"
#include <daily/timer/instrument.h>

static const int kNumEntities = TEST_SIZE;
static const float kTestLength = 10.0f;
static const float kFrameTime = 0.016f;

#define BOOST_TEST_MODULE Performance
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( manual_entity )
{
	std::vector<float> positions;
	std::vector<float> velocities;
	std::vector<float> accels;

	std::clog << "Created Pools\n";

	{	 DAILY_AUTO_INSTRUMENT_NODE(ComponentCreation);

		for(int i = 0; i < kNumEntities; ++i)
		{
			positions.push_back(0);
			velocities.push_back(0);
			accels.push_back(9.8f);
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
			for(std::size_t i = 0, s = accels.size(); i < s; ++i)
			{
				// Add a little to accel each frame.
				accels[i] += 0.001f;
			}

			for(std::size_t i = 0, s = velocities.size(); i < s; ++i)
			{
				// Compute new velocity.
				velocities[i] += (accels[i]/2.f) * (kFrameTime * kFrameTime);
			}

			for(std::size_t i = 0, s = positions.size(); i < s; ++i)
			{
				// Compute new position.
				positions[i] += velocities[i] * kFrameTime;
			}

			time_remaining -= kFrameTime;
		}
	}

	std::clog << "done.\n";

    std::clog << "Positions: " << positions[0] << std::endl;
    std::clog << "Velocities: " << velocities[0] << std::endl;

	if(!daily::timer_map::get_default().empty())
	{
		std::cout << "---------- Report -----------\n";
		daily::timer_map::get_default().gather_report(
			std::ostream_iterator<daily::timer_map::result_type>(std::cout, "\n")
		);
	}

	std::cout.flush();
}
