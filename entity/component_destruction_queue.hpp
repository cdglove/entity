//! \file entity/component_destruction_queue.h
//
// Represents a way to queue component destruction in order to 
// reduce creation complexity from O(m*nlog(n)) to O(m+n)
// 
#pragma once
#ifndef _ENTITY_COMPONENTDESTRUCTIONQUEUE_H_INCLUDED_
#define _ENTITY_COMPONENTDESTRUCTIONQUEUE_H_INCLUDED_

#include "entity/config.hpp" //IWYU pragma: keep
#include "entity/entity.hpp"
#include <algorithm>
#include <vector>
#include <utility>

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