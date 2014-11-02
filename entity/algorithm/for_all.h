//! \file entity/for_all.h
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not haver that component type, nullptr
// is passed instead.
// 
#pragma once
#ifndef _ENTITY_FORALL_H_INCLUDED_
#define _ENTITY_FORALL_H_INCLUDED_

#include "entity/config.h"
#include "entity/entity_component_iterator.h"
#include <algorithm>

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolTuple, typename Fn>
	void for_all(EntityList const& entities, ComponentPoolTuple&& p, Fn f)
	{
		auto i = begin(entities, std::forward<ComponentPoolTuple>(p));
		auto e = end(entities, std::forward<ComponentPoolTuple>(p));
		for(; i != e; ++i)
		{
			boost::fusion::invoke(f, *i);
		}
	}
}

#endif // _ENTITY_FORALL_H_INCLUDED_
