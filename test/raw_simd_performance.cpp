#include <iostream>
#include <vector>
#include <algorithm>
#include <immintrin.h>
#include "performance_common.h"

static const int kNumEntities = TEST_SIZE;
static const float kTestLength = 10.0f;
static const float kFrameTime = 0.016f;

int main()
{
#if USE_EXPLICIT_LOADS
	std::vector<unsigned int> ids;
	std::vector<float> positions;
	std::vector<float> velocities;
	std::vector<float> accels;

	ids.resize(kNumEntities);
	positions.resize(kNumEntities);
	velocities.resize(kNumEntities);
	accels.resize(kNumEntities);

	std::clog << "Created Pools\n";

	for(std::size_t i = 0; i < positions.size(); ++i)
	{
		ids[i] = i;
		accels[i] = 9.8f;
		velocities[i] = 0;
		positions[i] = 0;
	}

	std::clog << "Created Components, simulating...";

	// Simulate over some seconds using a fixed step.
	{
		float time_remaining = kTestLength;
		while(time_remaining > 0)
		{
			for(std::size_t i = 0; i < accels.size(); i += 4)
			{
				// Add a little to accel each frame.
				//if(ids[i] == i)
				{
					__m128 increment = _mm_set1_ps(0.001f);
					__m128 a = _mm_loadu_ps(&accels[i]);
					a += increment;
					_mm_stream_ps(&accels[i], a);
				}
			}

			for(std::size_t i = 0; i < velocities.size(); i += 4)
			{
				//if(ids[i] == i)
				{
					// Compute new velocity.
					__m128 divisor = _mm_set1_ps(2.f);
					__m128 frame_time_sq = _mm_set1_ps(kFrameTime * kFrameTime);
					__m128 a = _mm_loadu_ps(&accels[i]);
					__m128 v = _mm_loadu_ps(&velocities[i]);
					v += (a/divisor) * frame_time_sq;
					_mm_stream_ps(&velocities[i], v);
				}
			}

			for(std::size_t i = 0; i < positions.size(); i += 4)
			{
				//if(ids[i] == i)
				{
					// Compute new position.
					__m128 frame_time = _mm_set1_ps(kFrameTime);
					__m128 v = _mm_loadu_ps(&velocities[i]);
					__m128 p = _mm_loadu_ps(&positions[i]);
					p += v * frame_time;
					_mm_stream_ps(&positions[i], p);
				}
			}

			time_remaining -= kFrameTime;
		}
	}

	std::clog << "done.\n";

   	std::clog << "Positions: " << positions[0] << std::endl;
   	std::clog << "Velocities: " << velocities[0] << std::endl;
#else
   	std::vector<int> ids;
	std::vector<__m128> positions;
	std::vector<__m128> velocities;
	std::vector<__m128> accels;

	ids.resize(kNumEntities * 4);
	positions.resize(kNumEntities);
	velocities.resize(kNumEntities);
	accels.resize(kNumEntities);

	std::clog << "Created Pools\n";

	for(std::size_t i = 0; i < positions.size(); ++i)
	{
		ids[i] = i;
		accels[i] = _mm_set1_ps(9.8f);
		velocities[i] = _mm_setzero_ps();
		positions[i] = _mm_setzero_ps();
	}

	std::clog << "Created Components, simulating...";

	// Simulate over some seconds using a fixed step.
	{
		float time_remaining = kTestLength;
		while(time_remaining > 0)
		{
			for(std::size_t i = 0; i < accels.size(); ++i)
			{
				//if(ids[i] == i)
				{
					// Add a little to accel each frame.
					__m128 increment = _mm_set1_ps(0.001f);
					accels[i] += increment;
				}
			}

			for(std::size_t i = 0; i < velocities.size(); ++i)
			{
				//if(ids[i] == i)
				{
					// Compute new velocity.
					__m128 divisor = _mm_set1_ps(2.f);
					__m128 frame_time_sq = _mm_set1_ps(kFrameTime * kFrameTime);
					velocities[i] += (accels[i]/divisor) * frame_time_sq;
				}
			}

			for(std::size_t i = 0; i < positions.size(); ++i)
			{
				//if(ids[i] == i)
				{
					// Compute new position.
					__m128 frame_time = _mm_set1_ps(kFrameTime);
					positions[i] += velocities[i] * frame_time;
				}
			}

			time_remaining -= kFrameTime;
		}
	}

	std::clog << "done.\n";

   	std::clog << "Positions: " << _mm_cvtss_f32(positions[0]) << std::endl;
    std::clog << "Velocities: " << _mm_cvtss_f32(velocities[0]) << std::endl;
#endif

	return 0;
}






















