// ****************************************************************************
// entity/for_each.h
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not have all of the supplied compnent types
// f is not called.
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#ifndef _ENTITY_FUNCTIONAL_WINDOW_H_INCLUDED_
#define _ENTITY_FUNCTIONAL_WINDOW_H_INCLUDED_

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/entity.hpp"

// ----------------------------------------------------------------------------
//
namespace entity { namespace functional
{
	// ------------------------------------------------------------------------
	//
	struct get_window
	{
		template<typename ComponentPool>
		typename ComponentPool::window operator()(ComponentPool& p) const
		{
			return p.view();
		}
	};

	struct get_component
	{
		template<typename ComponentPoolWindow>
		typename ComponentPoolWindow::value_type& operator()(ComponentPoolWindow view) const
		{
			return view.get();
		}
	};

	struct increment_window
	{
		increment_window(entity target)
			: target_(target)
		{}

		template<typename ComponentPoolWindow>
		bool operator()(bool result, ComponentPoolWindow& view) const
		{
			return view.increment(target_) && result;
		}

		entity target_;
	};

	struct advance_window
	{
		advance_window(entity target)
			: target_(target)
		{}

		template<typename ComponentPoolWindow>
		bool operator()(bool result, ComponentPoolWindow& view) const
		{
			return view.increment(target_) && result;
		}

		entity target_;
	};
	
	struct is_entity
	{
		is_entity(entity target)
			: target_(target)
		{}

		template<typename ComponentPoolWindow>
		bool operator()(ComponentPoolWindow const& view) const
		{
			return view.is_entity(target_);
		}

		entity target_;
	};
	
	struct is_end
	{
		template<typename ComponentPoolWindow>
		bool operator()(ComponentPoolWindow const& view) const
		{
			return view.is_end();
		}
	};
}}

#endif // _ENTITY_FUNCTIONAL_WINDOW_H_INCLUDED_