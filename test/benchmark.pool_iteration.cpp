// ****************************************************************************
// test/benchmark.pool_iteration.cpp
//
// benchmarks the varoious ways that we can interate over the pools.
// 
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************

#include "entity/all.hpp"
#include "benchmark/benchmark.h"
#include <boost/iterator/zip_iterator.hpp>

#include "entity/component/detail/get_helper.hpp"
#include "entity/iterator/zip_iterator.hpp"

// -----------------------------------------------------------------------------
//
static const float kFrameTime = 0.016f;

// -----------------------------------------------------------------------------
//
struct jerk
{
	template<typename T>
	void operator()(T a)
	{
		if(a)
			*a += (0.001f *  kFrameTime);
	}
};

// -----------------------------------------------------------------------------
//
struct accelerate
{
	template<typename T>
	void operator()(T av)
	{
		using std::get;
		using boost::get;
		auto a = get<0>(av);
		auto v = get<1>(av);
		if(a && v)
			*v += *a * kFrameTime;
	}
};

// -----------------------------------------------------------------------------
//
struct move
{
	template<typename T>
	void operator()(T vp)
	{
		using std::get;
		using boost::get;
		auto v = get<0>(vp);
		auto p = get<1>(vp);
		if(v && p)
			*p += *v * kFrameTime;
	}
};

// -----------------------------------------------------------------------------
//
template<typename ComponentPool>
class ComponentFixture : public benchmark::Fixture
{
public:

	typedef ComponentPool pool_type;

	ComponentFixture()
		: accel_pool(entities)
		, velocity_pool(entities)
		, position_pool(entities)
	{}

	void SetUp(benchmark::State const& st) override
	{
		int num_entities = st.range_x();

		for (int i = 0; i < num_entities; ++i)
		{
			entities.create();
		}

		for(auto const& e : entities)
		{
			*position_pool.create(e, 0.f) = 0.f;
			*velocity_pool.create(e, 0.f) = 0.f;
			*accel_pool.create(e, 0.f) = 9.8f;
		}
	}

	void IterateRaw(benchmark::State& st)
	{
		while (st.KeepRunning())
		{
			float* a = &*accel_pool.get(entity::make_entity(0));
			//#pragma loop(no_vector)
			for(std::size_t i = 0, s = entities.size(); i < s; ++i)
			{
				a[i] += 0.001f * kFrameTime;
			}

			float* v = &*velocity_pool.get(entity::make_entity(0));
			//#pragma loop(no_vector)
			for(std::size_t i = 0, s = entities.size(); i < s; ++i)
			{
				v[i] += a[i] * kFrameTime;
			}

			float* p = &*position_pool.get(entity::make_entity(0));
			//#pragma loop(no_vector)
			for(std::size_t i = 0, s = entities.size(); i < s; ++i)
			{
				p[i] += v[i] * kFrameTime;
			}
		}
	}

	void IterateIndexed(benchmark::State& st)
	{
		while (st.KeepRunning())
		{
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = accel_pool.get(entity::make_entity(i));
				if(accel)
					*accel += 0.001f * kFrameTime;
			}

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = accel_pool.get(entity::make_entity(i));
				auto velocity = velocity_pool.get(entity::make_entity(i));
				if(accel && velocity)
					*velocity += *accel * kFrameTime;
			}

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto velocity = velocity_pool.get(entity::make_entity(i));
				auto position = position_pool.get(entity::make_entity(i));	
				if(velocity && position)
					*position += *velocity * kFrameTime;
			}
		}
	}

	void IterateUnchecked(benchmark::State& st)
	{
		while (st.KeepRunning())
		{
			auto accel_begin = accel_pool.begin();
			auto accel_end = accel_pool.end();
			for(auto accel = accel_begin; accel != accel_end; ++accel)
			{
				*accel += 0.001f * kFrameTime;
			}

			auto velocity_begin = velocity_pool.begin();
			auto velocity_end = velocity_pool.end();
			for(auto accel = accel_begin, velocity = velocity_begin; velocity != velocity_end; ++velocity, ++accel)
			{
				*velocity += *accel * kFrameTime;
			}

			auto position_begin = position_pool.begin();
			auto position_end = position_pool.end();
			for(auto velocity = velocity_begin, position = position_begin; position != position_end; ++position, ++velocity)
			{
				*position += *velocity * kFrameTime;
			}
		}
	}

	void IterateGetHelper(benchmark::State& st)
	{
		while (st.KeepRunning())
		{
			auto a_getter = entity::component::detail::make_get_helper(accel_pool);
			auto v_getter = entity::component::detail::make_get_helper(velocity_pool);
			auto p_getter = entity::component::detail::make_get_helper(position_pool);

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = a_getter.get(entity::make_entity(i));
				if(accel)
					*accel += 0.001f * kFrameTime;
			}

			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto accel = a_getter.get(entity::make_entity(i));
				auto velocity = v_getter.get(entity::make_entity(i));
				if(accel && velocity)
					*velocity += *accel * kFrameTime;
			}

			float* p = &*position_pool.get(entity::make_entity(0));
			for(entity::entity_index_t i = 0, s = entities.size(); i < s; ++i)
			{
				auto velocity = v_getter.get(entity::make_entity(i));
				auto position = p_getter.get(entity::make_entity(i));
				if(velocity && position)
					*position += *velocity * kFrameTime;
			}
		}
	}

	void IterateZip(benchmark::State& st)
	{
		while (st.KeepRunning())
		{
			std::for_each(
				accel_pool.optional_begin(),
				accel_pool.optional_end(),
				jerk()
			);

			std::for_each(
				entity::iterator::make_zip_iterator(entities.begin(), accel_pool, velocity_pool),
				entity::iterator::make_zip_iterator(entities.end(), accel_pool, velocity_pool),
				accelerate()
			);

			std::for_each(
				entity::iterator::make_zip_iterator(entities.begin(), velocity_pool, position_pool),
				entity::iterator::make_zip_iterator(entities.end(), velocity_pool, position_pool),
				move()
			);
		}
	}

	void IterateRange(benchmark::State& st)
	{
		while (st.KeepRunning())
		{
			auto ar = entity::range::make_optional_range(accel_pool);
			std::for_each(ar.begin(), ar.end(), jerk());

			auto avr = entity::range::combine(entities, accel_pool, velocity_pool);
			std::for_each(avr.begin(), avr.end(), accelerate());

			auto vpr = entity::range::combine(entities, velocity_pool, position_pool);
			std::for_each(vpr.begin(), vpr.end(), move());
		}
	}

	void IterateOptional(benchmark::State& st)
	{
		while (st.KeepRunning())
		{
			std::for_each(
				accel_pool.optional_begin(),
				accel_pool.optional_end(),
				jerk()
			);

			auto av_begin = boost::make_zip_iterator(boost::make_tuple(accel_pool.optional_begin(), velocity_pool.optional_begin()));
			auto av_end = boost::make_zip_iterator(boost::make_tuple(accel_pool.optional_end(), velocity_pool.optional_end()));
			std::for_each(
				av_begin,
				av_end,
				accelerate()
			);

			auto vp_begin = boost::make_zip_iterator(boost::make_tuple(velocity_pool.optional_begin(), position_pool.optional_begin()));
			auto vp_end = boost::make_zip_iterator(boost::make_tuple(velocity_pool.optional_end(), position_pool.optional_end()));
			std::for_each(
				vp_begin,
				vp_end,
				move()
			);
		}
	}

private:

	entity::entity_pool entities;
	ComponentPool accel_pool;
	ComponentPool velocity_pool;
	ComponentPool position_pool;
};

// Benchmark macros can't handle templates, so we'll use typdefs.
typedef ComponentFixture<entity::component::saturated_pool<float>> SaturatedFixture;
typedef ComponentFixture<entity::component::dense_pool<float>> DenseFixture;
typedef ComponentFixture<entity::component::sparse_pool<float>> SparseFixture;

// Add new pool types here.
#define POOLS				\
	POOL(SaturatedFixture)	\
	POOL(DenseFixture)		\
	POOL(SparseFixture)		\

// Add new tests here.
#define COMMON_TESTS(pool)					\
	TEST(IterateRaw, pool)					\
	TEST(IterateIndexed, pool)				\
	TEST(IterateUnchecked, pool)			\
	TEST(IterateGetHelper, pool)			\
	TEST(IterateZip, pool)					\
	TEST(IterateRange, pool)				\
	TEST(IterateOptional, pool)				\

// -----------------------------------------------------------------------------
// Auto instantiate tests here.
#ifdef _DEBUG
#  define BM_REGISTER(Fixture, Test) BENCHMARK_REGISTER_F(Fixture, Test)->Arg(1024)
#else
#  define BM_REGISTER(Fixture, Test) BENCHMARK_REGISTER_F(Fixture, Test)->Arg(1024)->Arg(1024 * 2048)
#endif

#define POOL(p) COMMON_TESTS(p)
#define TEST(t, p) \
	BENCHMARK_DEFINE_F(p, t)(benchmark::State& st)	\
	{												\
		t(st);										\
	}												\
	BM_REGISTER(p, t);								\

// 
POOLS

#undef TEST
#undef POOL
