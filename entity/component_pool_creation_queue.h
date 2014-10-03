//! \file entity/component_pool_creation_queue.h
//
// Represents a way to queue component creation in order to 
// reduce creation complexity from O(m*nlog(n)) to O(m+n)
// 

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

		template<typename... Args>
		void push(entity e, Args&&... args)
		{
			m_Created.push_back(std::make_pair(e, type(std::forward<Args>(args)...)));
		}

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