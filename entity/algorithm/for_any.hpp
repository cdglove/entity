// ****************************************************************************
// entity/algotithm/for_any.h
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not have any of the supplied components
// f is not called.
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef _ENTITY_ALGORITHM_FOREACH_H_INCLUDED_
#define _ENTITY_ALGORITHM_FOREACH_H_INCLUDED_

#include <boost/fusion/algorithm/iteration/fold.hpp>
#include <boost/fusion/algorithm/query/all.hpp>
#include <boost/fusion/algorithm/query/any.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <algorithm>

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/entity_range.hpp"
#include "entity/functional/window.hpp"
#include "entity/traits/iterator_traits.hpp"

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolTuple, typename Fn>
	void for_any(EntityList const& entities, ComponentPoolTuple&& p, iterator_traits::is_incremental_tag, Fn f)
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
			if(boost::fusion::any(c, functional::is_entity(*i)))
			{
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(c, functional::get_component_ptr())
				);
			}

			++i;
		}

		for(; i != e; ++i)
		{
			if(boost::fusion::fold(c, false, functional::increment_window_or(*i)))
			{
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(c, functional::get_component_ptr())
				);
			}	
		}
	}

	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPoolTuple, typename Fn>
	void for_any(EntityList const& entities, ComponentPoolTuple&& p, iterator_traits::is_skipping_tag, Fn f)
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
			if(boost::fusion::any(c, functional::is_entity(*i)))
			{
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(c, functional::get_component_ptr())
				);
			}

			++i;
		}

		for(; i != e; ++i)
		{
			if(boost::fusion::fold(c, false, functional::advance_window_or(*i)))
			{
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(c, functional::get_component_ptr())
				);
			}	
		}
	}

	template<typename EntityList, typename ComponentPoolTuple, typename Fn>
	void for_any(EntityList const& entities, ComponentPoolTuple&& p, Fn f)
	{
		for_any(entities, p, iterator_traits::entity_list_is_incremental<EntityList>(), f);
	}
}

#endif // _ENTITY_ALGORITHM_FOREACH_H_INCLUDED_
