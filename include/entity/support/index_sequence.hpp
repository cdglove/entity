// ****************************************************************************
// entity/support/variadic.hpp
//
// std::index_sequence implementation from c++14
//
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef ENTITY_SUPPORT_VARIADIC_H_INCLUDED_
#define ENTITY_SUPPORT_VARIADIC_H_INCLUDED_

namespace entity { namespace support {

// ------------------------------------------------------------------------
// 
template<std::size_t ... Indices>
struct index_sequence
{};

namespace detail 
{
	template<std::size_t new_index, typename Indices = index_sequence<>>
	struct make_indices_impl;

	template<std::size_t new_index, std::size_t... existing_indices>
	struct make_indices_impl<new_index, index_sequence<existing_indices...>>
	{
		typedef typename make_indices_impl<
			new_index - 1,
			index_sequence<new_index - 1, existing_indices...>
		>::type type;
	};

	template<std::size_t... existing_indices>
	struct make_indices_impl<0, index_sequence<existing_indices...>>
	{
		typedef index_sequence<existing_indices...> type;
	};
}

template<size_t num_indices>
static typename detail::make_indices_impl<num_indices>::type make_index_sequence()
{
	return typename detail::make_indices_impl<num_indices>::type();
}

} } // namespace entity { namespace support { 

#endif // ENTITY_SUPPORT_VARIADIC_H_INCLUDED_