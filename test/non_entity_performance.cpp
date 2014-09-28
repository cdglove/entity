#include "entity/dense_component_pool.h"
#include "entity/sparse_component_pool.h"
#include "entity/algorithm/for_each.h"
#include "entity/algorithm/for_all.h"
#include "entity/component_pool_creation_queue.h"
#include "entity/component_pool_destruction_queue.h"

#include <random>
#include <iostream>

#include <daily/timer/instrument.h>

static const int kNumEntities = 1024 * 2048;
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

int main()
{
	std::vector<PhysicsData> entities;
	entities.resize(kNumEntities);

	std::clog << "Created Pools\n";

	unsigned int ids = 0;
	std::for_each(entities.begin(), entities.end(), [&ids](PhysicsData& p)
	{
		p.id = ids++;
		p.accel = 9.8f;
	});

	std::clog << "Created Components, simulating...";

	// Simulate over some seconds using a fixed step.
	{
		float time_remaining = kTestLength;
		while(time_remaining > 0)
		{
			std::for_each(entities.begin(), entities.end(), [](PhysicsData& p)
			{
				// Add a little to accel each frame.
				p.accel += 0.001f;
			});

			std::for_each(entities.begin(), entities.end(), [](PhysicsData& p)
			{
				// Compute new velocity.
				p.velocity += (p.accel/2.f) * (kFrameTime * kFrameTime);
			});

			std::for_each(entities.begin(), entities.end(), [](PhysicsData& p)
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

	return 0;
}
