// ****************************************************************************
// entity/support/mutable_pair.hpp
//
// Represents a pair with a mutable second so it can be used within 
// the boost::multi_index container.
//
// Take from boost.multi_index examples available here:
//
// http://www.boost.org/doc/libs/1_60_0/libs/multi_index/doc/tutorial/techniques.html#emulate_std_containers
// 
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef ENTITY_SUPPORT_MUTABLEPAIR_H_INCLUDED_
#define ENTITY_SUPPORT_MUTABLEPAIR_H_INCLUDED_

// -----------------------------------------------------------------------------
//
namespace entity { namespace support {

template <typename T1,typename T2>
struct mutable_pair
{
	typedef T1 first_type;
	typedef T2 second_type;

	mutable_pair():first(T1()),second(T2()){}
	mutable_pair(const T1& f,const T2& s):first(f),second(s){}
	mutable_pair(const std::pair<T1,T2>& p):first(p.first),second(p.second){}

	T1         first;
	mutable T2 second;
};

template<typename T1, typename T2>
mutable_pair<T1, T2> make_mutable_pair(T1&& t1, T2&& t2)
{
	return mutable_pair<T1, T2>(std::forward<T1>(t1), std::forward<T2>(t2));
}

}}

#endif // ENTITY_SUPPORT_MUTABLEPAIR_H_INCLUDED_