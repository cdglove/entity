// ****************************************************************************
// test/benchmark.base_iteration.cpp
//
// Benchmarks for non-pool iteration. As a baseline.
// 
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************

#include "benchmark/benchmark.h"
#include <vector>
#include <algorithm>

// -----------------------------------------------------------------------------
//
static const float kFrameTime = 0.016f;

template<typename I, typename Fn>
void for_each(I i, I e, Fn f)
{
	#pragma loop( no_vector )
	for(; i != e; ++i)
	{
		f(*i);
	}
}

// -----------------------------------------------------------------------------
//
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
std::vector<PhysicsData> entities;

static void StructIteration(benchmark::State& st) 
{
	int num_entities = st.range_x();

	unsigned int ids = 0;
	for(int i = 0; i < num_entities; ++i)
	{
		PhysicsData p;
		p.id = ids++;
		p.accel = 9.8f;
		p.velocity = 0;
		p.position = 0;
		entities.push_back(p);
	}

	while(st.KeepRunning())
	{
		::for_each(entities.begin(), entities.end(),
			[](PhysicsData& p)
			{
				p.accel += 0.001f;
			}
		);

		::for_each(entities.begin(), entities.end(), 
			[](PhysicsData& p)
			{
				p.velocity += p.accel * kFrameTime;
			}
		);

		::for_each(entities.begin(), entities.end(), 
			[](PhysicsData& p)
			{
				p.position += p.velocity * kFrameTime;
			}
		);
	}

	benchmark::DoNotOptimize(entities);
}
BENCHMARK(StructIteration)->Arg(1024)->Arg(1024 * 2048);

// -----------------------------------------------------------------------------
// Leave these in globals otherwise the compiler optimizes things out on me.
std::vector<float> positions;
std::vector<float> velocities;
std::vector<float> accels;

static void ManualComponentIteration(benchmark::State& st) 
{
	int num_entities = st.range_x();

	for(int i = 0; i < num_entities; ++i)
	{
		positions.push_back(0);
		velocities.push_back(0);
		accels.push_back(9.8f);
	}

	while(st.KeepRunning())
	{
		for(std::size_t i = 0, s = accels.size(); i < s; ++i)
		{
			accels[i] += 0.001f;
		}

		for(std::size_t i = 0, s = velocities.size(); i < s; ++i)
		{
			velocities[i] += accels[i] * kFrameTime;
		}

		for(std::size_t i = 0, s = positions.size(); i < s; ++i)
		{
			positions[i] += velocities[i] * kFrameTime;
		}
	}

	benchmark::DoNotOptimize(positions);
	benchmark::DoNotOptimize(velocities);
	benchmark::DoNotOptimize(accels);
}
BENCHMARK(ManualComponentIteration)->Arg(1024)->Arg(1024 * 2048);

