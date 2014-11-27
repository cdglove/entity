//! \file entity/component_pool_creation_queue.h
//
// Represents a way to queue component creation in order to 
// reduce creation complexity from O(m*nlog(n)) to O(m+n)
// 
#pragma once
#ifndef _ENTITY_COMPONENTPOOLCREATIONQUEUE_H_INCLUDED_
#define _ENTITY_COMPONENTPOOLCREATIONQUEUE_H_INCLUDED_

#include "entity/config.h"
#include <vector>
#include <utility>

// ----------------------------------------------------------------------------
//
namespace entity
{
	template<typename ComponentPool>
	class component_pool_creation_queue
	{
	public:

		typedef typename ComponentPool::type type;

		component_pool_creation_queue(ComponentPool& p)
			: m_Pool(p)
		{}

		~component_pool_creation_queue()
		{
			flush();
		}

	#if ENTITY_SUPPORT_VARIADICS
		template<typename... Args>
		void push(entity e, Args&&... args)
		{
			m_Created.push_back(std::make_pair(e, type(std::forward<Args>(args)...)));
		}
	#else
		void push(entity e, type&& original)
		{
			m_Created.push_back(std::make_pair(e, std::move(original)));
		}
	#endif

		void flush()
		{
			std::sort(m_Created.begin(), m_Created.end());
			m_Pool.create_range(m_Created.begin(), m_Created.end());
			clear();
		}

		void clear()
		{
			m_Created.clear();
		}

	private: 

		// No copying.
		component_pool_creation_queue(component_pool_creation_queue const&);
		component_pool_creation_queue operator=(component_pool_creation_queue);

		std::vector<std::pair<entity, type>> m_Created;
		ComponentPool& m_Pool;
	};
}

#endif // _ENTITY_COMPONENTPOOLCREATIONQUEUE_H_INCLUDED_