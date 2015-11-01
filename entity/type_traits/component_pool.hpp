// ****************************************************************************
// entity/type_traits/component_pool.hpp
//
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef _ENTITY_TYPETRAITS_COMPONENTPOOL_H_INCLUDED_
#define _ENTITY_TYPETRAITS_COMPONENTPOOL_H_INCLUDED_

namespace entity { namespace type_traits {

template<typename ComponentPool>
struct optional_type_of_pool
{
	typedef typename ComponentPool::optional_type type;
};

template<typename ComponentPool>
struct optional_type_of_pool<ComponentPool const>
{
	typedef typename ComponentPool::const_optional_type type;
};


} } // namespace entity { namespace type_traits {
#endif // _ENTITY_TYPETRAITS_COMPONENTPOOL_H_INCLUDED_