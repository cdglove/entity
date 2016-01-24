// ****************************************************************************
// entity/component/creation_queue.h
//
// Represents a way to queue component creation in order to 
// reduce creation complexity from O(m*nlog(n)) to O(m+n)
//
// Copyright Chris Glover 2014-2016
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// ****************************************************************************
#pragma once
#ifndef ENTITY_COMPONENT_CREATIONQUEUE_H_INCLUDED_
#define ENTITY_COMPONENT_CREATIONQUEUE_H_INCLUDED_

#include "entity/config.hpp" // IWYU pragma: keep
#include "entity/entity.hpp"
#include <algorithm>
#include <vector>
#include <utility>

// ----------------------------------------------------------------------------
//
namespace entity { namespace component
{
	template<typename ComponentPool>
	class creation_queue
	{
	public:

		typedef typename ComponentPool::type type;

		creation_queue(ComponentPool& p)
			: pool_(p)
		{}

		~creation_queue()
		{
			flush();
		}

		template<typename... Args>
		void push(weak_entity e, Args&&... args)
		{
			created_.push_back(std::make_pair(e, type(std::forward<Args>(args)...)));
		}
	
		void flush()
		{
			std::sort(created_.begin(), created_.end());
			pool_.create_range(created_.begin(), created_.end());
			clear();
		}

		void clear()
		{
			created_.clear();
		}

	private: 

		// No copying.
		creation_queue(creation_queue const&);
		creation_queue operator=(creation_queue);

		std::vector<std::pair<weak_entity, type>> created_;
		ComponentPool& pool_;
	};
} } // namespace entity { namespace component

#endif // ENTITY_COMPONENT_CREATIONQUEUE_H_INCLUDED_