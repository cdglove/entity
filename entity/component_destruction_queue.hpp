// ****************************************************************************
// entity/component_destruction_queue.h
//
// Represents a way to queue component destruction in order to 
// reduce creation complexity from O(m*nlog(n)) to O(m+n)
// 
// Copyright Chris Glover 2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef _ENTITY_COMPONENTDESTRUCTIONQUEUE_H_INCLUDED_
#define _ENTITY_COMPONENTDESTRUCTIONQUEUE_H_INCLUDED_

#include <algorithm>
#include <functional>
#include <vector>

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/entity.hpp"

// ----------------------------------------------------------------------------
//
namespace entity
{
	template<typename ComponentPool>
	class component_destruction_queue
	{
	public:

		typedef typename ComponentPool::type type;

		component_destruction_queue(ComponentPool& p)
			: pool_(p)
		{}

		~component_destruction_queue()
		{
			flush();
		}

		void push(weak_entity e)
		{
			destroyed_.push_back(e);
		}

		void flush()
		{
			std::sort(destroyed_.begin(), destroyed_.end(), std::greater<weak_entity>());
			pool_.destroy_range(destroyed_.begin(), destroyed_.end());
			clear();
		}

		void clear()
		{
			destroyed_.clear();
		}

	private: 

		// No copying.
		component_destruction_queue(component_destruction_queue const&);
		component_destruction_queue operator=(component_destruction_queue);

		std::vector<weak_entity> destroyed_;
		ComponentPool& pool_;
	};
}

#endif // _ENTITY_COMPONENTDESTRUCTIONQUEUE_H_INCLUDED_
