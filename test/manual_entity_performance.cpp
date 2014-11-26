#define DAILY_ENABLE_INSTRUMENTATION 0
#include <iostream>
#include <vector>
#include <algorithm>
#include <daily/timer/instrument.h>

#if DAILY_ENABLE_INSTRUMENTATION
#  define SIZE_OF_TEST 1024 * 256
#endif

#if SIZE_OF_TEST
	static const int kNumEntities = SIZE_OF_TEST;
#else
	static const int kNumEntities = 1024 * 2048;
#endif 

static const float kTestLength = 10.0f;
static const float kFrameTime = 0.016f;

template<typename I, typename Fn>
void for_each(I i, I e, Fn f)
{
	for(; i != e; ++i)
	{
		f(*i);
	}
}

int main()
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
		DAILY_AUTO_INSTRUMENT_NODE(Simulation);

		float time_remaining = kTestLength;
		while(time_remaining > 0)
		{
			for(int i = 0; i < kNumEntities; ++i)
			{
				// Add a little to accel each frame.
				accels[i] += 0.001f;
			}

			for(int i = 0; i < kNumEntities; ++i)
			{
				// Compute new velocity.
				velocities[i] += (accels[i]/2.f) * (kFrameTime * kFrameTime);
			}

			for(int i = 0; i < kNumEntities; ++i)
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

	std::cout << "---------- Report -----------\n";
	daily::timer_map::get_default().report(std::cout);
	std::cout.flush();

	return 0;
}
