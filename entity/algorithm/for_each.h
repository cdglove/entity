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
#include <tuple>

#if 0 //ENTITY_SUPPORT_VARIADICS == 0
#  include <boost/proprocessor.hpp>
#endif

// ----------------------------------------------------------------------------
//
namespace entity
{
#if ENTITY_SUPPORT_VARIADICS
	template<typename... ComponentPools>
	class entity_component_pool_package
	{
	public:

		entity_component_pool_package(ComponentPools... pools)
			: components_(std::forward<ComponentPools>(pools)...)
		{}

	private:

		std::tuple<ComponentPools...> components_;
	};
	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename... ComponentPools>
	void for_each(EntityList const& entities, ComponentPools&&... pools)
	{
		// using boost::fusion::at_c;
		// for(auto i = begin(entities, pools...), e = end(entities, pools...); i != e; ++i)
		// {
		// 	auto const& components = *i;
		// 	// if(at_c<1>(components))
		// 	// {
		// 	// 	f(*at_c<1>(components));
		// 	// }
		// }
	}
#else
	// ------------------------------------------------------------------------
	//
	template<typename EntityList, typename ComponentPool>
	void for_each(EntityList const& entities, ComponentPool& p, void(f)(typename ComponentPool::type&))
	{
		using boost::fusion::at_c;
		for(auto i = begin(entities, p), e = end(entities, p); i != e; ++i)
		{
			auto const& components = *i;
			if(at_c<1>(components))
			{
				f(*at_c<1>(components));
			}
		}
	}

	template<typename EntityList, typename ComponentPool1, typename ComponentPool2, typename Fn>
	void for_each(EntityList const& entities, ComponentPool1& p1, ComponentPool2& p2, Fn f)
	{
		using boost::fusion::at_c;
		for(auto i = begin(entities, p1, p2), e = end(entities, p1, p2); i != e; ++i)
		{
			auto const& components = *i;
			if(at_c<1>(components) && at_c<2>(components))
			{
				f(*at_c<1>(components), *at_c<2>(components));
			}
		}
	}
#endif
}

#endif // _COMPONENT_FOREACH_H_INCLUDED_