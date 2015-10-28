// ****************************************************************************
// entity/component/detail/get_helper.hpp
//
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef _ENTITY_COMPONENT_GETHELPER_H_INCLUDED_
#define _ENTITY_COMPONENT_GETHELPER_H_INCLUDED_

// ----------------------------------------------------------------------------
//
namespace entity { namespace component { namespace detail {

template<typename ComponentPool>
class get_helper
{
public:

	typedef typename ComponentPool::optional_type optional_type;

	get_helper(ComponentPool& pool)
		: pool_(&pool)
	{}

	optional_type get(entity e)
	{
		return pool_->get(e);
	}

	optional_type get(entity e) const
	{
		return pool_->get(e);
	}

private:

	ComponentPool* pool_;

};

template<typename ComponentPool>
get_helper<ComponentPool> make_get_helper(ComponentPool& pool)
{
	return get_helper<ComponentPool>(pool);
}

} } } // namespace entity { namespace component { namespace detail {
#endif // _ENTITY_COMPONENT_GETHELPER_H_INCLUDED_