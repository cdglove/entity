#define DAILY_ENABLE_INSTRUMENTATION 0
#include <iostream>
#include <vector>
#include <algorithm>
#include <daily/timer/instrument.h>
#include "performance_common.h"

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

int main()
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
		DAILY_AUTO_INSTRUMENT_NODE(Simulation);

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
		daily::timer_map::get_default().report(std::cout);
	}

	std::cout.flush();

	return 0;
}
