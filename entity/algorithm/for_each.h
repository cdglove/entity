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
#include <boost/fusion/include/as_vector.hpp>
#include <algorithm>

// ----------------------------------------------------------------------------
//
namespace entity
{
	// ------------------------------------------------------------------------
	//
	namespace detail
	{
		struct get_view
		{
			template<typename ComponentPool>
			typename ComponentPool::range operator()(ComponentPool& p) const
			{
				return p.view();
			}
		};

		struct get_component
		{
			get_component(entity owner)
				: owner_(owner)
			{}

			template<typename ComponentPoolView>
			typename ComponentPoolView::type& operator()(ComponentPoolView view) const
			{
				return view.get(owner_);
			}

			entity owner_;
		};

		struct advance_view
		{
			advance_view(entity target)
				: target_(target)
			{}

			template<typename ComponentPoolView>
			void operator()(ComponentPoolView& view) const
			{
				view.advance(target_);
			}

			entity target_;
		};

		struct is_valid_view
		{
			is_valid_view(entity target)
				: target_(target)
			{}

			template<typename ComponentPoolView>
			bool operator()(ComponentPoolView view) const
			{
				return view.is_valid(target_);
			}

			entity target_;
		};
	};

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
				detail::get_view()
			)
		);

		for(; i != e; ++i)
		{
			DAILY_AUTO_INSTRUMENT_NODE(foreach_invoke);
			entity ent = *i;
			boost::fusion::for_each(c, detail::advance_view(ent));
			if(boost::fusion::all(c, detail::is_valid_view(ent)))
			{
				boost::fusion::invoke(
					f, 
					boost::fusion::transform(c, detail::get_component(ent))
				);
			}	
		}
	}
}

#endif // _ENTITY_ALGORITHM_FOREACH_H_INCLUDED_