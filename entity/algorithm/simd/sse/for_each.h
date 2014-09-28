//! \file entity/ForEach.h
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not have all of the supplied compnent types
// f is not called.
// 
#pragma once
#ifndef _COMPONENT_SIMD_SSE_FOREACH_H_INCLUDED_
#define _COMPONENT_SIMD_SSE_FOREACH_H_INCLUDED_

#include <algorithm>
#include <immintrin.h>
#include "entity/entity_component_iterator.h"

// ----------------------------------------------------------------------------
//
namespace entity { namespace simd { namespace sse
{
	template<typename EntityList, typename ComponentPool, typename Fn>
	void for_each(EntityList const& entities, ComponentPool& p, Fn f)
	{
		using boost::fusion::at_c;
		for(auto i = begin(entities, p), e = end(entities, p); i != e; std::advance(i, 4))
		{
			auto const& components = *i;
			float* a_float = &*at_c<1>(components);
			__m128 a = _mm_loadu_ps(a_float);
			f(a);
			_mm_storeu_ps(a_float, a);
		}
	}

	template<typename EntityList, typename ComponentPool1, typename ComponentPool2, typename Fn>
	void for_each(EntityList const& entities, ComponentPool1& p1, ComponentPool2& p2, Fn f)
	{
		using boost::fusion::at_c;
		for(auto i = begin(entities, p1, p2), e = end(entities, p1, p2); i != e; std::advance(i, 4))
		{
			auto const& components = *i;
			float* a_float = &*at_c<1>(components);
			float* b_float = &*at_c<2>(components);
			__m128 a = _mm_loadu_ps(a_float);
			__m128 b = _mm_loadu_ps(b_float);
			f(a, b);
			_mm_storeu_ps(b_float, b);
			_mm_storeu_ps(a_float, a);
		}
	}
}}} // namespace entity { namespace simd { namespace sse { 

#endif // _COMPONENT_SIMD_SSE_FOREACH_H_INCLUDED_