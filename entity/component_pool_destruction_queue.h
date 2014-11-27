//! \file entity/component_pool_destruction_queue.h
//
// Represents a way to queue component destruction in order to 
// reduce creation complexity from O(m*nlog(n)) to O(m+n)
// 
#pragma once
#ifndef _ENTITY_COMPONENTPOOLDESTRUCTIONQUEUE_H_INCLUDED_
#define _ENTITY_COMPONENTPOOLDESTRUCTIONQUEUE_H_INCLUDED_

#include "entity/config.h"
#include <vector>
#include <utility>

// ----------------------------------------------------------------------------
//
namespace entity
{
	template<typename ComponentPool>
	class component_pool_destruction_queue
	{
	public:

		typedef typename ComponentPool::type type;

		component_pool_destruction_queue(ComponentPool& p)
			: pool_(p)
		{}

		~component_pool_destruction_queue()
		{
			flush();
		}

		void push(entity e)
		{
			destroyed_.push_back(e);
		}

		void flush()
		{
			std::sort(destroyed_.begin(), destroyed_.end(), std::greater<entity>());
			pool_.destroy_range(destroyed_.begin(), destroyed_.end());
			clear();
		}

		void clear()
		{
			destroyed_.clear();
		}

	private: 

		// No copying.
		component_pool_destruction_queue(component_pool_destruction_queue const&);
		component_pool_destruction_queue operator=(component_pool_destruction_queue);

		std::vector<entity> destroyed_;
		ComponentPool& pool_;
	};
}

#endif // _ENTITY_COMPONENTPOOLDESTRUCTIONQUEUE_H_INCLUDED_