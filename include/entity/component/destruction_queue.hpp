// ****************************************************************************
// entity/component/destruction_queue.h
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
#ifndef ENTITY_COMPONENT_DESTRUCTIONQUEUE_H_INCLUDED_
#define ENTITY_COMPONENT_DESTRUCTIONQUEUE_H_INCLUDED_

#include <algorithm>
#include <functional>
#include <vector>

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/entity.hpp"

// ----------------------------------------------------------------------------
//
namespace entity { namespace component 
{
	template<typename ComponentPool>
	class destruction_queue
	{
	public:

		typedef typename ComponentPool::type type;

		destruction_queue(ComponentPool& p)
			: pool_(p)
		{}

		~destruction_queue()
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
		destruction_queue(destruction_queue const&);
		destruction_queue operator=(destruction_queue);

		std::vector<weak_entity> destroyed_;
		ComponentPool& pool_;
	};
} } // namespace entity { namespace component {

#endif // ENTITY_COMPONENT_DESTRUCTIONQUEUE_H_INCLUDED_
