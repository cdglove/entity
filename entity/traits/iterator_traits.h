// ****************************************************************************
// entity/traits/iterator_traits.h
//
// Traits about various pool types, etc.
// 
// Copyright Chris Glover 2014
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef _ENTITY_TRAITS_ITERATORTRAITS_H_INCLUDED_
#define _ENTITY_TRAITS_ITERATORTRAITS_H_INCLUDED_

namespace entity
{
	namespace iterator_traits
	{
		struct is_incremental_tag {};
		struct is_skipping_tag {};

		template<typename EntityList>
		struct entity_list_is_incremental : is_skipping_tag
		{};
	}

	iterator_traits::is_incremental_tag const incremental{};
	iterator_traits::is_skipping_tag const skipping{};
}

#endif // _ENTITY_TRAITS_ITERATORTRAITS_H_INCLUDED_