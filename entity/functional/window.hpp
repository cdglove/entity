//! \file entity/for_each.h
//
// Algorithm to call a functor for an entity with the supplied component
// types.  If the entity does not have all of the supplied compnent types
// f is not called.
// 
#pragma once
#ifndef _ENTITY_FUNCTIONAL_WINDOW_H_INCLUDED_
#define _ENTITY_FUNCTIONAL_WINDOW_H_INCLUDED_

#include "entity/config.hpp" //IWYU pragma: keep
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
		bool operator()(ComponentPoolWindow& view) const
		{
			return view.is_entity(target_);
		}

		entity target_;
	};
}}

#endif // _ENTITY_FUNCTIONAL_WINDOW_H_INCLUDED_