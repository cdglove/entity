// ****************************************************************************
// entity/algotithm/simd/sse/for_each.hpp
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not have all of the supplied compnent types
// f is not called.
// This version is optimized for sse such that the functor f will be supplied
// _m128s filled with floats. Clearly, this implies float components.
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef _ENTITY_ALGORITHM_SIMD_SSE_FOREACH_H_INCLUDED_
#define _ENTITY_ALGORITHM_SIMD_SSE_FOREACH_H_INCLUDED_

#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <xmmintrin.h>

#include "entity/algorithm/simd/detail/invoke.hpp"
#include "entity/config.hpp" // IWYU pragma: keep

// ----------------------------------------------------------------------------
//
namespace entity { namespace simd { namespace sse
{
	namespace detail
	{
		struct loadu_ps
		{
			inline void operator()(boost::fusion::vector<float*, __m128&> source_and_v) const
			{
				using boost::fusion::at_c;
				at_c<1>(source_and_v) = _mm_loadu_ps(at_c<0>(source_and_v));
			}
		};

		struct storeu_ps
		{
			inline void operator()(boost::fusion::vector<float*, __m128&> dest_and_v) const
			{
				using boost::fusion::at_c;
				_mm_storeu_ps(at_c<0>(dest_and_v), at_c<1>(dest_and_v));
			}
		};

		struct iterator_to_ptr
		{
			template<typename Iterator>
			float* operator()(Iterator iter) const
			{
				return &(*iter);
			}
		};
	}
	
	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolTuple, typename Fn>
	void for_each(EntityList const& entities, ComponentPoolTuple p, Fn f)
	{
		//DAILY_AUTO_INSTRUMENT_NODE(for_each);
		//typedef typename boost::mpl::transform<
		//	ComponentPoolTuple,
		//	simd::detail::make_type<boost::mpl::_1, __m128>
		//>::type m128_holder;

		//typedef typename boost::mpl::transform<
		//	m128_holder,
		//	boost::add_reference<boost::mpl::_1>
		//>::type m128_refs;

		//m128_holder data;
		//m128_refs data_refs(data);

		//auto i = begin(entities, std::forward<ComponentPoolTuple>(p));
		//auto e = end(entities, std::forward<ComponentPoolTuple>(p));

		//while(i != e)
		//{
		//	boost::fusion::for_each(
		//		boost::fusion::zip(
		//			boost::fusion::transform(*i, detail::iterator_to_ptr()),
		//			data_refs
		//		),
		//		detail::loadu_ps()
		//	);

		//	simd::detail::invoke(f, data_refs);

		//	boost::fusion::for_each(
		//		boost::fusion::zip(
		//			boost::fusion::transform(*i, detail::iterator_to_ptr()),
		//			data_refs
		//		),
		//		detail::storeu_ps()
		//	);

		//	for(int j = 0; j < 4; ++j)
		//	{
		//		DAILY_AUTO_INSTRUMENT_NODE(for_each_loop);
		//		++i;
		//		if(i == e)
		//		{
		//			break;
		//		}
		//	}
		//}
	}
}}} // namespace entity { namespace simd { namespace sse { 

#endif // _ENTITY_ALGORITHM_SIMD_SSE_FOREACH_H_INCLUDED_
