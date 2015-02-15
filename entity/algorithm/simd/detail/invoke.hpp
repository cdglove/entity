// ****************************************************************************
// entity/algorithm/simd/detail/invoke.h
//
// Invoker helper function for simd to workaround limitations
// of boost::fusion::invoke
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef _ENTITY_ALGORITHM_SIMD_DETAIL_INVOKE_H_INCLUDED_
#define _ENTITY_ALGORITHM_SIMD_DETAIL_INVOKE_H_INCLUDED_

#include <boost/fusion/sequence/intrinsic/at.hpp>

#include "entity/config.hpp" // IWYU pragma: keep

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
