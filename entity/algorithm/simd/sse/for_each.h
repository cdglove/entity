//! \file entity/algorithm/simd/sse/for_each.h
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not have all of the supplied compnent types
// f is not called.
// 
#pragma once
#ifndef _ENTITY_ALGORITHM_SIMD_SSE_FOREACH_H_INCLUDED_
#define _ENTITY_ALGORITHM_SIMD_SSE_FOREACH_H_INCLUDED_

#include <algorithm>
#include <immintrin.h>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/algorithm/transformation/zip.hpp>
#include "entity/entity_component_iterator.h"
#include "entity/functional/dereference.h"
#include "entity/algorithm/simd/detail/invoke.h"

// ----------------------------------------------------------------------------
//
namespace entity { namespace simd { namespace sse
{
	namespace detail
	{
		struct loadu_ps
		{
			template<typename V>
			void operator()(V const& source_and_v) const
			{
				using boost::fusion::at_c;
				at_c<1>(source_and_v) = _mm_loadu_ps(&*at_c<0>(source_and_v));
			}
		};

		struct storeu_ps
		{
			template<typename V>
			void operator()(V const& dest_and_v) const
			{
				using boost::fusion::at_c;
				_mm_storeu_ps(&*at_c<0>(dest_and_v), at_c<1>(dest_and_v));
			}
		};
	}
	
	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolTuple, typename Fn>
	void for_each(EntityList const& entities, ComponentPoolTuple&& p, Fn f)
	{
		typedef typename boost::mpl::transform<
			ComponentPoolTuple,
			simd::detail::make_type<boost::mpl::_1, __m128>
		>::type m128_holder;

		typedef typename boost::mpl::transform<
			m128_holder,
			boost::add_reference<boost::mpl::_1>
		>::type m128_refs;

		m128_holder data;
		m128_refs data_refs(data);

		auto i = begin(entities, std::forward<ComponentPoolTuple>(p));
		auto e = end(entities, std::forward<ComponentPoolTuple>(p));

		for(; i != e; std::advance(i, 4))
		{
			
			boost::fusion::for_each(
				boost::fusion::zip(
					*i,
					data_refs
				),
				detail::loadu_ps()
			);

			simd::detail::invoke(f, data_refs);

			boost::fusion::for_each(
				boost::fusion::zip(
					*i,
					data_refs
				),
				detail::storeu_ps()
			);
		}
	}
}}} // namespace entity { namespace simd { namespace sse { 

#endif // _ENTITY_ALGORITHM_SIMD_SSE_FOREACH_H_INCLUDED_
