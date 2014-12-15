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
#include "entity/functional/window.h"
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/algorithm/query/all.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <algorithm>

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	template<typename ComponentPoolTuple, typename Fn>
	void for_each(entity_pool const& entities, ComponentPoolTuple&& p, Fn f)
	{
		auto i = begin(entities); 
		auto e = end(entities);

		auto c = boost::fusion::as_vector(
			boost::fusion::transform(
				std::forward<ComponentPoolTuple>(p),
				functional::get_window()
			)
		);

		if(i != e)
		{
			if(boost::fusion::all(c, functional::is_entity(*i)))
			{
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(c, functional::get_component())
				);
			}

			++i;
		}

		for(; i != e; ++i)
		{
			if(boost::fusion::fold(c, true, functional::increment_window(*i)))
			{
				DAILY_AUTO_INSTRUMENT_NODE(foreach_invoke);
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(c, functional::get_component())
				);
			}	
		}
	}

	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolTuple, typename Fn>
	void for_each(EntityList const& entities, ComponentPoolTuple&& p, Fn f)
	{
		auto i = begin(entities); 
		auto e = end(entities);

		auto c = boost::fusion::as_vector(
			boost::fusion::transform(
			std::forward<ComponentPoolTuple>(p),
			functional::get_window()
			)
		);

		if(i != e)
		{
			if(boost::fusion::all(c, functional::is_entity(*i)))
			{
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(c, functional::get_component())
				);
			}

			++i;
		}

		for(; i != e; ++i)
		{
			if(boost::fusion::fold(c, true, functional::advance_window(*i)))
			{
				DAILY_AUTO_INSTRUMENT_NODE(foreach_invoke);
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(c, functional::get_component())
				);
			}	
		}
	}
}

#endif // _ENTITY_ALGORITHM_FOREACH_H_INCLUDED_