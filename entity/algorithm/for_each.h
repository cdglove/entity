//! \file entity/ForEach.h
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not have all of the supplied compnent types
// f is not called.
// 
#pragma once
#ifndef _COMPONENT_FOREACH_H_INCLUDED_
#define _COMPONENT_FOREACH_H_INCLUDED_

#include "entity/config.h"
#include "entity/entity_component_iterator.h"
#include <algorithm>

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename... ComponentPools, typename Fn) //void(f)(boost::fusion::vector<typename ComponentPools::type* ...> const&)>
	void for_each(EntityList const& entities, ComponentPools&... p, Fn f)
	{
		//using boost::fusion::at_c;
		for(auto i = begin(entities, tie(p...)), e = end(entities, tie(p...)); i != e; ++i)
		{
			//f(*i);
		}
	}
}

#endif // _COMPONENT_FOREACH_H_INCLUDED_