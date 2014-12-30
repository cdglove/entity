//! \file entity/algorithm/simd/detail/invoke.h
//
// Invoker helper function for simd to workaround limitations
// of boost::fusion::invoke
// 
#pragma once
#ifndef _ENTITY_ALGORITHM_SIMD_DETAIL_INVOKE_H_INCLUDED_
#define _ENTITY_ALGORITHM_SIMD_DETAIL_INVOKE_H_INCLUDED_

#include "entity/config.hpp" // IWYU pragma: keep
#include <immintrin.h>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>

// ----------------------------------------------------------------------------
//
namespace entity { namespace simd { namespace detail
{	
	template<typename Source, typename Target>
	struct make_type
	{
		typedef Target type;
	};

	template<typename Fn, template<typename> class Sequence, typename intrinsic_type>
	void invoke(Fn f, Sequence<intrinsic_type&>& values)
	{
		using boost::fusion::at_c;
		f(at_c<0>(values));
	}

	template<typename Fn, template<typename, typename> class Sequence, typename intrinsic_type>
	void invoke(Fn f, Sequence<intrinsic_type&, intrinsic_type&>& values)
	{
		using boost::fusion::at_c;
		f(at_c<0>(values), at_c<1>(values));
	}

	template<typename Fn, template<typename, typename, typename> class Sequence, typename intrinsic_type>
	void invoke(Fn f, Sequence<intrinsic_type&, intrinsic_type&, intrinsic_type&>& values)
	{
		using boost::fusion::at_c;
		f(at_c<0>(values), at_c<1>(values), at_c<2>(values));
	}

	template<typename Fn, template<typename, typename, typename, typename> class Sequence, typename intrinsic_type>
	void invoke(Fn f, Sequence<intrinsic_type&, intrinsic_type&, intrinsic_type&, intrinsic_type&>& values)
	{
		using boost::fusion::at_c;
		f(at_c<0>(values), at_c<1>(values), at_c<2>(values), at_c<3>(values));
	}
}}}

#endif // _ENTITY_ALGORITHM_SIMD_DETAIL_INVOKE_H_INCLUDED_