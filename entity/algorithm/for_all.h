//! \file entity/ForAll.h
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not haver that component type, nullptr
// is passed instead.
// 
#pragma once
#ifndef _COMPONENT_FORALL_H_INCLUDED_
#define _COMPONENT_FORALL_H_INCLUDED_

#include "entity/config.h"
#include "entity/entity_component_iterator.h"
#include <algorithm>

// ----------------------------------------------------------------------------
//
namespace entity
{
	template<typename EntityList, typename ComponentPool, typename Fn>
	void for_all(EntityList const& entities, ComponentPool& p, Fn f)
	{
		using boost::fusion::at_c;
		for(auto i = begin(entities, p), e = end(entities, p); i != e; ++i)
		{
			auto const& components = *i;
			f(at_c<1>(components));
		}
	}

	template<typename EntityList, typename ComponentPool1, typename ComponentPool2, typename Fn>
	void for_all(EntityList const& entities, ComponentPool1& p1, ComponentPool2& p2, Fn f)
	{
		using boost::fusion::at_c;
		for(auto i = begin(entities, p1, p2), e = end(entities, p1, p2); i != e; ++i)
		{
			auto const& components = *i;
			f(at_c<1>(components),at_c<2>(components));
		}
	}
}

#endif // _COMPONENT_FORALL_H_INCLUDED_