//! \file entity/for_each.h
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not have all of the supplied compnent types
// f is not called.
// 
#pragma once
#ifndef _ENTITY_ALGORITHM_FOREACH_H_INCLUDED_
#define _ENTITY_ALGORITHM_FOREACH_H_INCLUDED_

#include "entity/config.h"
#include "entity/entity_component_iterator.h"
#include "entity/functional/is_valid_component.h"
#include "entity/functional/dereference.h"
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/algorithm/query/all.hpp>
#include <algorithm>

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolTuple, typename Fn>
	void for_each(EntityList const& entities, ComponentPoolTuple&& p, Fn f)
	{
		auto i = begin(entities, std::forward<ComponentPoolTuple>(p));
		auto e = end(entities, std::forward<ComponentPoolTuple>(p));
		for(; i != e; ++i)
		{
			if(boost::fusion::all(*i, is_valid_component()))
			{
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(*i, dereference())
				);
			}
		}
	}
}

#endif // _ENTITY_ALGORITHM_FOREACH_H_INCLUDED_